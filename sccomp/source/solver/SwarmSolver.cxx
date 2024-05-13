/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <rtl/math.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <comphelper/compbase.hxx>
#include <comphelper/propertycontainer2.hxx>
#include <comphelper/proparrhlp.hxx>

#include <cmath>
#include <vector>
#include <limits>
#include <chrono>
#include <random>

#include <unotools/resmgr.hxx>

#include "DifferentialEvolution.hxx"
#include "ParticelSwarmOptimization.hxx"

#include <strings.hrc>

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

enum
{
    PROP_NONNEGATIVE,
    PROP_INTEGER,
    PROP_TIMEOUT,
    PROP_ALGORITHM,
};

} // end anonymous namespace

typedef comphelper::WeakImplHelper<sheet::XSolver, sheet::XSolverDescription, lang::XServiceInfo>
    SwarmSolver_Base;

namespace
{
class SwarmSolver : public comphelper::OPropertyContainer2,
                    public comphelper::OPropertyArrayUsageHelper<SwarmSolver>,
                    public SwarmSolver_Base
{
private:
    uno::Reference<sheet::XSpreadsheetDocument> mxDocument;
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

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    virtual uno::Reference<beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return createPropertySetInfo(getInfoHelper());
    }
    // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper& getInfoHelper() override { return *getArrayHelper(); }
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

    virtual sal_Bool SAL_CALL getMaximize() override { return mbMaximize; }
    virtual void SAL_CALL setMaximize(sal_Bool bMaximize) override { mbMaximize = bMaximize; }

    virtual sal_Bool SAL_CALL getSuccess() override { return mbSuccess; }
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

    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
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
    uno::Reference<container::XIndexAccess> xSheets(mxDocument->getSheets(), uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByIndex(rPosition.Sheet),
                                               uno::UNO_QUERY);
    return xSheet->getCellByPosition(rPosition.Column, rPosition.Row);
}

void SwarmSolver::setValue(const table::CellAddress& rPosition, double fValue)
{
    getCell(rPosition)->setValue(fValue);
}

double SwarmSolver::getValue(const table::CellAddress& rPosition)
{
    return getCell(rPosition)->getValue();
}

IMPLEMENT_FORWARD_XINTERFACE2(SwarmSolver, SwarmSolver_Base, comphelper::OPropertyContainer2)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(SwarmSolver, SwarmSolver_Base, comphelper::OPropertyContainer2)

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

    if (doesViolateConstraints())
        return std::numeric_limits<float>::lowest();

    double x = getValue(maObjective);

    if (mbMaximize)
        return x;
    else
        return -x;
}

void SwarmSolver::initializeVariables(std::vector<double>& rVariables, std::mt19937& rGenerator)
{
    int nTry = 1;
    bool bConstraintsOK = false;

    while (!bConstraintsOK && nTry < 10)
    {
        size_t noVariables(maVariables.getLength());

        rVariables.resize(noVariables);

        for (size_t i = 0; i < noVariables; ++i)
        {
            Bound const& rBound = maBounds[i];
            if (mbInteger)
            {
                sal_Int64 intLower(rBound.lower);
                sal_Int64 intUpper(rBound.upper);
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
    double fResult = std::clamp(fValue, rBound.lower, rBound.upper);

    if (mbInteger)
        return std::trunc(fResult);

    return fResult;
}

double SwarmSolver::boundVariable(size_t nVarIndex, double fValue)
{
    Bound const& rBound = maBounds[nVarIndex];
    // double fResult = std::max(std::min(fValue, rBound.upper), rBound.lower);
    double fResult = fValue;
    while (fResult < rBound.lower || fResult > rBound.upper)
    {
        if (fResult < rBound.lower)
            fResult = rBound.upper - (rBound.lower - fResult);
        if (fResult > rBound.upper)
            fResult = (fResult - rBound.upper) + rBound.lower;
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
            return false;
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

namespace
{
template <typename SwarmAlgorithm> class SwarmRunner
{
private:
    SwarmAlgorithm& mrAlgorithm;
    double mfTimeout;

    static constexpr size_t mnPopulationSize = 40;
    static constexpr int constNumberOfGenerationsWithoutChange = 50;

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

        int nLastChange = 0;

        while ((mrAlgorithm.getGeneration() - nLastChange) < constNumberOfGenerationsWithoutChange
               && duration_cast<milliseconds>(maEnd - maStart).count() < mfTimeout)
        {
            bool bChange = mrAlgorithm.next();

            if (bChange)
                nLastChange = mrAlgorithm.getGeneration();

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

    maBounds.resize(maVariables.getLength());

    xModel->lockControllers();

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

    std::vector<double> aSolution;

    if (mnAlgorithm == 0)
    {
        DifferentialEvolutionAlgorithm<SwarmSolver> aDE(*this, 50);
        SwarmRunner<DifferentialEvolutionAlgorithm<SwarmSolver>> aEvolution(aDE);
        aEvolution.setTimeout(mnTimeout);
        aSolution = aEvolution.solve();
    }
    else
    {
        ParticleSwarmOptimizationAlgorithm<SwarmSolver> aPSO(*this, 100);
        SwarmRunner<ParticleSwarmOptimizationAlgorithm<SwarmSolver>> aSwarmSolver(aPSO);
        aSwarmSolver.setTimeout(mnTimeout);
        aSolution = aSwarmSolver.solve();
    }

    xModel->unlockControllers();

    mbSuccess = true;

    maSolution.realloc(aSolution.size());
    std::copy(aSolution.begin(), aSolution.end(), maSolution.getArray());
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Calc_SwarmSolver_get_implementation(uno::XComponentContext*,
                                                      uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new SwarmSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
