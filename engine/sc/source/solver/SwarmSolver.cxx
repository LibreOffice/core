/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <rtl/math.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <comphelper/propcontainerimplhelper.hxx>
#include <comphelper/scopeguard.hxx>

#include <cmath>
#include <vector>
#include <limits>
#include <chrono>
#include <random>
#include <unordered_map>

#include <o3tl/hash_combine.hxx>

#include <unotools/resmgr.hxx>
#include <comphelper/servicehelper.hxx>

#include <docuno.hxx>
#include <document.hxx>
#include <address.hxx>

#include "DifferentialEvolution.hxx"
#include "ParticelSwarmOptimization.hxx"

#include "strings.hrc"

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace css;

namespace
{
struct Bound
{
    double lower;
    double upper;

    Bound()
        // float bounds should be low/high enough for all practical uses
        // otherwise we are too far away from the solution
        : lower(std::numeric_limits<float>::lowest())
        , upper(std::numeric_limits<float>::max())
    {
    }

    void updateBound(sheet::SolverConstraintOperator eOp, double fValue)
    {
        if (eOp == sheet::SolverConstraintOperator_LESS_EQUAL)
        {
            // if we set the bound multiple times use the one which includes both values
            // for example bound values 100, 120, 150 -> use 100 -> the lowest one
            if (fValue < upper)
                upper = fValue;
        }
        else if (eOp == sheet::SolverConstraintOperator_GREATER_EQUAL)
        {
            if (fValue > lower)
                lower = fValue;
        }
        else if (eOp == sheet::SolverConstraintOperator_EQUAL)
        {
            lower = fValue;
            upper = fValue;
        }
    }
};

// A cell position used as a cache key.
struct CellKey
{
    sal_Int32 nSheet;
    sal_Int32 nColumn;
    sal_Int32 nRow;

    bool operator==(const CellKey& rOther) const
    {
        return nSheet == rOther.nSheet && nColumn == rOther.nColumn && nRow == rOther.nRow;
    }
};

struct CellKeyHash
{
    size_t operator()(const CellKey& rKey) const
    {
        size_t nSeed = 0;
        o3tl::hash_combine(nSeed, rKey.nSheet);
        o3tl::hash_combine(nSeed, rKey.nColumn);
        o3tl::hash_combine(nSeed, rKey.nRow);
        return nSeed;
    }
};

enum
{
    PROP_NONNEGATIVE,
    PROP_INTEGER,
    PROP_TIMEOUT,
    PROP_ALGORITHM,
};

} // end anonymous namespace

namespace
{
class SwarmSolver
    : public comphelper::OPropertyContainerImplHelper<
          comphelper::WeakImplHelper<sheet::XSolver, sheet::XSolverDescription, lang::XServiceInfo>,
          SwarmSolver>
{
private:
    uno::Reference<sheet::XSpreadsheetDocument> mxDocument;
    // The ScDocument implementation behind mxDocument
    ScDocument* mpDocument = nullptr;
    table::CellAddress maObjective;
    uno::Sequence<table::CellAddress> maVariables;
    uno::Sequence<sheet::SolverConstraint> maConstraints;
    bool mbMaximize;

    // set via XPropertySet
    bool mbNonNegative;
    bool mbInteger;
    sal_Int32 mnTimeout;
    sal_Int32 mnAlgorithm;

    // results
    bool mbSuccess;
    double mfResultValue;

    uno::Sequence<double> maSolution;
    OUString maStatus;

    std::vector<Bound> maBounds;
    std::vector<sheet::SolverConstraint> maNonBoundedConstraints;

    // The document's variable values at the start of a solve, clamped into the
    // bounds. Empty until solve captures it.
    std::vector<double> maStartingValues;
    // How many times variables have been initialized this solve.
    size_t mnInitCount = 0;
    // Number of population members seeded from maStartingValues. Set from the
    // population size when a solve starts.
    size_t mnSeedCount = 0;

    // Resolved cells, keyed by sheet, column and row. The search reads and
    // writes the same handful of cells many thousands of times, so resolving
    // each one only once is a large saving.
    std::unordered_map<CellKey, uno::Reference<table::XCell>, CellKeyHash> maCellCache;

private:
    static OUString getResourceString(TranslateId aId);

    uno::Reference<table::XCell> getCell(const table::CellAddress& rPosition);
    void setValue(const table::CellAddress& rPosition, double fValue);
    double getValue(const table::CellAddress& rPosition);

public:
    SwarmSolver()
        : mbMaximize(true)
        , mbNonNegative(false)
        , mbInteger(false)
        , mnTimeout(60000)
        , mnAlgorithm(0)
        , mbSuccess(false)
        , mfResultValue(0.0)
    {
        registerProperty(u"NonNegative"_ustr, PROP_NONNEGATIVE, 0, &mbNonNegative,
                         cppu::UnoType<decltype(mbNonNegative)>::get());
        registerProperty(u"Integer"_ustr, PROP_INTEGER, 0, &mbInteger,
                         cppu::UnoType<decltype(mbInteger)>::get());
        registerProperty(u"Timeout"_ustr, PROP_TIMEOUT, 0, &mnTimeout,
                         cppu::UnoType<decltype(mnTimeout)>::get());
        registerProperty(u"Algorithm"_ustr, PROP_ALGORITHM, 0, &mnAlgorithm,
                         cppu::UnoType<decltype(mnAlgorithm)>::get());
    }

    // OPropertyArrayUsageHelper
    virtual cppu::IPropertyArrayHelper* createArrayHelper() const override
    {
        uno::Sequence<beans::Property> aProperties;
        describeProperties(aProperties);
        return new cppu::OPropertyArrayHelper(aProperties);
    }

    // XSolver
    virtual uno::Reference<sheet::XSpreadsheetDocument> SAL_CALL getDocument() override
    {
        return mxDocument;
    }
    virtual void SAL_CALL
    setDocument(const uno::Reference<sheet::XSpreadsheetDocument>& rDocument) override
    {
        mxDocument = rDocument;
        mpDocument = nullptr;
        if (ScModelObj* pModel = comphelper::getFromUnoTunnel<ScModelObj>(mxDocument))
        {
            mpDocument = pModel->GetDocument();
            // The solver writes input cells and reads the recalculated objective
            // and constraint cells, which needs automatic recalculation on.
            if (mpDocument)
                mpDocument->SetAutoCalc(true);
        }
    }

    virtual table::CellAddress SAL_CALL getObjective() override { return maObjective; }
    virtual void SAL_CALL setObjective(const table::CellAddress& rObjective) override
    {
        maObjective = rObjective;
    }

    virtual uno::Sequence<table::CellAddress> SAL_CALL getVariables() override
    {
        return maVariables;
    }
    virtual void SAL_CALL setVariables(const uno::Sequence<table::CellAddress>& rVariables) override
    {
        maVariables = rVariables;
    }

    virtual uno::Sequence<sheet::SolverConstraint> SAL_CALL getConstraints() override
    {
        return maConstraints;
    }
    virtual void SAL_CALL
    setConstraints(const uno::Sequence<sheet::SolverConstraint>& rConstraints) override
    {
        maConstraints = rConstraints;
    }

    virtual bool SAL_CALL getMaximize() override { return mbMaximize; }
    virtual void SAL_CALL setMaximize(bool bMaximize) override { mbMaximize = bMaximize; }

    virtual bool SAL_CALL getSuccess() override { return mbSuccess; }
    virtual double SAL_CALL getResultValue() override { return mfResultValue; }

    virtual uno::Sequence<double> SAL_CALL getSolution() override { return maSolution; }

    virtual void SAL_CALL solve() override;

    // XSolverDescription
    virtual OUString SAL_CALL getComponentDescription() override
    {
        return SwarmSolver::getResourceString(RID_SWARM_SOLVER_COMPONENT);
    }

    virtual OUString SAL_CALL getStatusDescription() override { return maStatus; }

    virtual OUString SAL_CALL getPropertyDescription(const OUString& rPropertyName) override
    {
        TranslateId pResId;
        switch (getInfoHelper().getHandleByName(rPropertyName))
        {
            case PROP_NONNEGATIVE:
                pResId = RID_PROPERTY_NONNEGATIVE;
                break;
            case PROP_INTEGER:
                pResId = RID_PROPERTY_INTEGER;
                break;
            case PROP_TIMEOUT:
                pResId = RID_PROPERTY_TIMEOUT;
                break;
            case PROP_ALGORITHM:
                pResId = RID_PROPERTY_ALGORITHM;
                break;
            default:
                break;
        }
        return SwarmSolver::getResourceString(pResId);
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.Calc.SwarmSolver"_ustr;
    }

    bool SAL_CALL supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.sheet.Solver"_ustr };
    }

private:
    void applyVariables(std::vector<double> const& rVariables);
    bool doesViolateConstraints();
    // Total amount by which the current cell values break the non-bounded
    // constraints, summed over all of them. Zero when every constraint holds.
    double constraintViolation();
    bool isSolutionFeasible(std::vector<double> const& rSolution);

public:
    double calculateFitness(std::vector<double> const& rVariables);
    size_t getDimensionality() const;
    void initializeVariables(std::vector<double>& rVariables, std::mt19937& rGenerator);
    double clampVariable(size_t nVarIndex, double fValue);
    double boundVariable(size_t nVarIndex, double fValue);
};
}

OUString SwarmSolver::getResourceString(TranslateId aId)
{
    if (!aId)
        return OUString();

    return Translate::get(aId, Translate::Create("scc"));
}

uno::Reference<table::XCell> SwarmSolver::getCell(const table::CellAddress& rPosition)
{
    CellKey aKey{ rPosition.Sheet, rPosition.Column, rPosition.Row };
    auto aFound = maCellCache.find(aKey);
    if (aFound != maCellCache.end())
        return aFound->second;

    uno::Reference<container::XIndexAccess> xSheets(mxDocument->getSheets(), uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByIndex(rPosition.Sheet),
                                               uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(rPosition.Column, rPosition.Row);
    maCellCache.emplace(aKey, xCell);
    return xCell;
}

// Build a document cell address from the API address, throwing if its sheet is
// outside the document. Access through the spreadsheet UNO objects used to throw
// on a missing sheet. Direct ScDocument access does not, so validate here to keep
// an invalid variable cell an error rather than a silently ignored write.
static ScAddress lcl_cellAddress(const ScDocument& rDocument, const table::CellAddress& rPosition)
{
    if (rPosition.Sheet < 0 || rPosition.Sheet >= rDocument.GetTableCount())
        throw uno::RuntimeException(u"solver: cell address is outside the document"_ustr);
    return ScAddress(static_cast<SCCOL>(rPosition.Column), static_cast<SCROW>(rPosition.Row),
                     static_cast<SCTAB>(rPosition.Sheet));
}

void SwarmSolver::setValue(const table::CellAddress& rPosition, double fValue)
{
    if (mpDocument)
        mpDocument->SetValue(lcl_cellAddress(*mpDocument, rPosition), fValue);
}

double SwarmSolver::getValue(const table::CellAddress& rPosition)
{
    if (!mpDocument)
        return 0.0;
    return mpDocument->GetValue(lcl_cellAddress(*mpDocument, rPosition));
}

void SwarmSolver::applyVariables(std::vector<double> const& rVariables)
{
    for (sal_Int32 i = 0; i < maVariables.getLength(); ++i)
    {
        setValue(maVariables[i], rVariables[i]);
    }
}

double SwarmSolver::calculateFitness(std::vector<double> const& rVariables)
{
    applyVariables(rVariables);

    double x = getValue(maObjective);
    double fFitness = mbMaximize ? x : -x;

    // Graded penalty: an infeasible candidate is pushed down in proportion to
    // how badly it breaks the constraints, so a point close to feasible scores
    // higher than one far off and the search has a slope to follow toward the
    // feasible region. The strict feasibility test still decides what is
    // reported as a solution, so a point that only nearly satisfies the
    // constraints is never returned as the answer.
    double fViolation = constraintViolation();
    if (fViolation > 0.0)
        fFitness -= 1.0e7 * fViolation;

    return fFitness;
}

double SwarmSolver::constraintViolation()
{
    double fTotal = 0.0;
    for (const sheet::SolverConstraint& rConstraint : maNonBoundedConstraints)
    {
        double fLeftValue = getValue(rConstraint.Left);
        double fRightValue = 0.0;

        table::CellAddress aCellAddress;

        if (rConstraint.Right >>= aCellAddress)
            fRightValue = getValue(aCellAddress);
        else if (rConstraint.Right >>= fRightValue)
        {
            // The right hand side is a plain number.
        }
        else
        {
            // The right hand side is neither a cell nor a number, so this
            // constraint cannot be evaluated. Skip it, matching the strict
            // feasibility check.
            continue;
        }

        switch (rConstraint.Operator)
        {
            case sheet::SolverConstraintOperator_LESS_EQUAL:
                if (fLeftValue > fRightValue)
                    fTotal += fLeftValue - fRightValue;
                break;
            case sheet::SolverConstraintOperator_GREATER_EQUAL:
                if (fLeftValue < fRightValue)
                    fTotal += fRightValue - fLeftValue;
                break;
            case sheet::SolverConstraintOperator_EQUAL:
                if (!rtl::math::approxEqual(fLeftValue, fRightValue))
                    fTotal += std::abs(fLeftValue - fRightValue);
                break;
            default:
                break;
        }
    }
    return fTotal;
}

void SwarmSolver::initializeVariables(std::vector<double>& rVariables, std::mt19937& rGenerator)
{
    size_t nVariables(maVariables.getLength());
    rVariables.resize(nVariables);

    size_t nIndex = mnInitCount++;

    // The first seeded individual is the document's starting values unchanged, a
    // feasible-scale anchor. The next seeded ones keep that scale but spread out
    // with jitter scaled to each value's magnitude, so the search has diversity
    // near the guess. Everything past mnSeedCount is drawn at random across the
    // bounds for global exploration.
    if (!maStartingValues.empty() && nIndex < mnSeedCount)
    {
        if (nIndex == 0)
        {
            for (size_t i = 0; i < nVariables; ++i)
                rVariables[i] = clampVariable(i, maStartingValues[i]);
            return;
        }
        std::normal_distribution<double> aJitter(0.0, 0.5);
        for (size_t i = 0; i < nVariables; ++i)
        {
            double fBase = maStartingValues[i];
            double fScale = std::max(1.0, std::abs(fBase));
            rVariables[i] = clampVariable(i, fBase + fScale * aJitter(rGenerator));
        }
        return;
    }

    int nTry = 1;
    bool bConstraintsOK = false;

    while (!bConstraintsOK && nTry < 10)
    {
        for (size_t i = 0; i < nVariables; ++i)
        {
            Bound const& rBound = maBounds[i];
            if (rBound.lower >= rBound.upper)
            {
                // An empty or reversed range has nothing to draw from. The
                // random distributions are undefined when the lower bound is
                // not below the upper one, so take the lower bound directly.
                rVariables[i] = rBound.lower;
            }
            else if (mbInteger)
            {
                // The default bounds sit around the float range, which a 64 bit
                // integer cannot hold. Turning such a double into sal_Int64 is
                // undefined, so clamp to a magnitude that converts safely first.
                // 2^62 is a power of two well inside the range and exact in both
                // types.
                constexpr double fIntegerLimit = double(sal_Int64(1) << 62);
                sal_Int64 intLower(std::clamp(rBound.lower, -fIntegerLimit, fIntegerLimit));
                sal_Int64 intUpper(std::clamp(rBound.upper, -fIntegerLimit, fIntegerLimit));
                std::uniform_int_distribution<sal_Int64> random(intLower, intUpper);
                rVariables[i] = double(random(rGenerator));
            }
            else
            {
                std::uniform_real_distribution<double> random(rBound.lower, rBound.upper);
                rVariables[i] = random(rGenerator);
            }
        }

        applyVariables(rVariables);

        bConstraintsOK = !doesViolateConstraints();
        nTry++;
    }
}

double SwarmSolver::clampVariable(size_t nVarIndex, double fValue)
{
    Bound const& rBound = maBounds[nVarIndex];

    // An empty or reversed range has no room to clamp into. std::clamp with a
    // lower bound above the upper bound is undefined, so return the lower bound.
    if (rBound.lower >= rBound.upper)
        return mbInteger ? std::trunc(rBound.lower) : rBound.lower;

    double fResult = std::clamp(fValue, rBound.lower, rBound.upper);

    if (mbInteger)
        return std::trunc(fResult);

    return fResult;
}

double SwarmSolver::boundVariable(size_t nVarIndex, double fValue)
{
    Bound const& rBound = maBounds[nVarIndex];

    double fResult = fValue;
    double const fRange = rBound.upper - rBound.lower;
    if (fRange <= 0.0)
    {
        // An empty range, where the lower and upper bound are equal, or a
        // reversed range from contradictory constraints, holds a single
        // value. Return the lower bound.
        fResult = rBound.lower;
    }
    else if (fResult < rBound.lower || fResult > rBound.upper)
    {
        // Wrap an out of range value back into the range, keeping its offset
        // from the lower bound.
        double fOffset = std::fmod(fResult - rBound.lower, fRange);
        if (fOffset < 0.0)
            fOffset += fRange;
        fResult = rBound.lower + fOffset;
    }

    if (mbInteger)
        return std::trunc(fResult);

    return fResult;
}

size_t SwarmSolver::getDimensionality() const { return maVariables.getLength(); }

bool SwarmSolver::doesViolateConstraints()
{
    for (const sheet::SolverConstraint& rConstraint : maNonBoundedConstraints)
    {
        double fLeftValue = getValue(rConstraint.Left);
        double fRightValue = 0.0;

        table::CellAddress aCellAddress;

        if (rConstraint.Right >>= aCellAddress)
        {
            fRightValue = getValue(aCellAddress);
        }
        else if (rConstraint.Right >>= fRightValue)
        {
            // empty
        }
        else
        {
            // The right hand side is neither a cell nor a number, so this
            // constraint cannot be evaluated. Skip it and carry on checking the
            // rest of the constraints.
            continue;
        }

        sheet::SolverConstraintOperator eOp = rConstraint.Operator;
        switch (eOp)
        {
            case sheet::SolverConstraintOperator_LESS_EQUAL:
            {
                if (fLeftValue > fRightValue)
                    return true;
            }
            break;
            case sheet::SolverConstraintOperator_GREATER_EQUAL:
            {
                if (fLeftValue < fRightValue)
                    return true;
            }
            break;
            case sheet::SolverConstraintOperator_EQUAL:
            {
                if (!rtl::math::approxEqual(fLeftValue, fRightValue))
                    return true;
            }
            break;
            default:
                break;
        }
    }
    return false;
}

bool SwarmSolver::isSolutionFeasible(std::vector<double> const& rSolution)
{
    // The search must have produced a value for every variable.
    if (sal_Int32(rSolution.size()) != maVariables.getLength())
        return false;

    // A value outside the variable's bound is not a valid solution.
    for (size_t i = 0; i < rSolution.size(); ++i)
    {
        Bound const& rBound = maBounds[i];
        if (rSolution[i] < rBound.lower || rSolution[i] > rBound.upper)
            return false;
    }

    // Put the candidate into the document and check the remaining constraints.
    applyVariables(rSolution);
    return !doesViolateConstraints();
}

namespace
{
template <typename SwarmAlgorithm> class SwarmRunner
{
private:
    SwarmAlgorithm& mrAlgorithm;
    double mfTimeout;

    static constexpr size_t mnPopulationSize = 40;
    static constexpr int constNumberOfGenerationsWithoutChange = 50;
    // How many fresh restarts to try after the search stalls before giving up.
    // An easy model converges and then stops after these few extra tries, while
    // a hard one keeps finding better points across restarts and runs on until
    // the time budget is gone.
    static constexpr int constMaxStallRestarts = 3;

    std::chrono::high_resolution_clock::time_point maStart;
    std::chrono::high_resolution_clock::time_point maEnd;

public:
    SwarmRunner(SwarmAlgorithm& rAlgorithm)
        : mrAlgorithm(rAlgorithm)
        , mfTimeout(5000)
    {
    }

    void setTimeout(double fTimeout) { mfTimeout = fTimeout; }

    std::vector<double> const& solve()
    {
        using std::chrono::duration_cast;
        using std::chrono::high_resolution_clock;
        using std::chrono::milliseconds;

        mrAlgorithm.initialize();

        maEnd = maStart = high_resolution_clock::now();

        int nStallRestarts = 0;
        double fBestSoFar = mrAlgorithm.getBestFitness();

        while (duration_cast<milliseconds>(maEnd - maStart).count() < mfTimeout)
        {
            mrAlgorithm.next();

            // Any real improvement in the best fitness earns back the full
            // restart budget, so a search that keeps making progress is never
            // cut off early.
            double fBest = mrAlgorithm.getBestFitness();
            if (fBest > fBestSoFar)
            {
                fBestSoFar = fBest;
                nStallRestarts = 0;
            }

            if ((mrAlgorithm.getGeneration() - mrAlgorithm.getLastChange())
                >= constNumberOfGenerationsWithoutChange)
            {
                // The search has settled. Stop once a few fresh restarts have
                // also failed to improve on the best point found.
                if (nStallRestarts >= constMaxStallRestarts)
                    break;
                nStallRestarts++;
                mrAlgorithm.restart();
            }

            maEnd = high_resolution_clock::now();
        }
        return mrAlgorithm.getResult();
    }
};
}

void SAL_CALL SwarmSolver::solve()
{
    uno::Reference<frame::XModel> xModel(mxDocument, uno::UNO_QUERY_THROW);

    maStatus.clear();
    mbSuccess = false;
    if (!maVariables.getLength())
        return;

    // Start each solve from fresh state. assign refills every bound with a
    // default, and clearing drops the constraints from the previous run, which
    // are only ever appended.
    maBounds.assign(maVariables.getLength(), Bound());
    maNonBoundedConstraints.clear();
    maCellCache.clear();

    xModel->lockControllers();

    // Unlock again on any exit from here on, including an exception thrown by
    // one of the cell accesses below, so the document is always left usable.
    comphelper::ScopeGuard aUnlockGuard([&xModel] { xModel->unlockControllers(); });

    if (mbNonNegative)
    {
        for (Bound& rBound : maBounds)
            rBound.lower = 0;
    }

    // Determine variable bounds
    for (sheet::SolverConstraint const& rConstraint : maConstraints)
    {
        table::CellAddress aLeftCellAddress = rConstraint.Left;
        sheet::SolverConstraintOperator eOp = rConstraint.Operator;

        size_t index = 0;
        bool bFoundVariable = false;
        for (const table::CellAddress& rVariableCell : maVariables)
        {
            if (aLeftCellAddress == rVariableCell)
            {
                bFoundVariable = true;
                table::CellAddress aCellAddress;
                double fValue;

                if (rConstraint.Right >>= aCellAddress)
                {
                    uno::Reference<table::XCell> xCell = getCell(aCellAddress);
                    if (xCell->getType() == table::CellContentType_VALUE)
                    {
                        maBounds[index].updateBound(eOp, xCell->getValue());
                    }
                    else
                    {
                        maNonBoundedConstraints.push_back(rConstraint);
                    }
                }
                else if (rConstraint.Right >>= fValue)
                {
                    maBounds[index].updateBound(eOp, fValue);
                }
            }
            index++;
        }
        if (!bFoundVariable)
            maNonBoundedConstraints.push_back(rConstraint);
    }

    // Capture the document's current variable values, clamped into the bounds,
    // as a starting guess to seed part of the initial population from.
    maStartingValues.assign(maVariables.getLength(), 0.0);
    for (sal_Int32 i = 0; i < maVariables.getLength(); ++i)
        maStartingValues[i] = clampVariable(i, getValue(maVariables[i]));
    mnInitCount = 0;

    // Scale the population with the number of variables. A fixed small
    // population cannot keep enough diversity on a high-dimensional model, while
    // the original sizes are kept for small models so their behaviour does not
    // change. The cap keeps each generation's cost bounded.
    const size_t nDimensions = maVariables.getLength();

    std::vector<double> aSolution;

    if (mnAlgorithm == 0)
    {
        size_t nPopulation = std::clamp<size_t>(10 * nDimensions, 50, 300);
        mnSeedCount = nPopulation / 2;
        DifferentialEvolutionSolver<SwarmSolver> aDE(*this, nPopulation);
        SwarmRunner<DifferentialEvolutionSolver<SwarmSolver>> aEvolution(aDE);
        aEvolution.setTimeout(mnTimeout);
        aSolution = aEvolution.solve();
    }
    else
    {
        size_t nPopulation = std::clamp<size_t>(10 * nDimensions, 100, 300);
        mnSeedCount = nPopulation / 2;
        ParticleSwarmOptimizationSolver<SwarmSolver> aPSO(*this, nPopulation);
        SwarmRunner<ParticleSwarmOptimizationSolver<SwarmSolver>> aSwarmSolver(aPSO);
        aSwarmSolver.setTimeout(mnTimeout);
        aSolution = aSwarmSolver.solve();
    }

    // Only report success when the returned values actually satisfy the
    // model. The search can run out of generations or time without ever
    // reaching a feasible point.
    bool bFeasible = isSolutionFeasible(aSolution);

    mbSuccess = bFeasible;

    if (bFeasible)
    {
        maSolution.realloc(aSolution.size());
        std::copy(aSolution.begin(), aSolution.end(), maSolution.getArray());
    }
    else
    {
        maSolution.realloc(0);
        maStatus = getResourceString(RID_ERROR_INFEASIBLE);
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Calc_SwarmSolver_get_implementation(uno::XComponentContext*,
                                                      uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new SwarmSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
