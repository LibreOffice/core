/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "CoinMPModel.hxx"
#include "SolverComponent.hxx"
#include "strings.hrc"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <comphelper/scopeguard.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>
#include <float.h>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace com::sun::star;

namespace
{
// Sequential Quadratic Programming (SQP) solver for smooth nonlinear models.
// CoinMP can only solve straight-line (linear) problems, so this solver reaches
// a nonlinear optimum by solving a sequence of small quadratic problems, each a
// local model of the real one around the latest point. The quadratic curvature
// lets it converge in far fewer steps than a purely linear model would.
//
// Starting from the current cell values, it repeats four steps until the point
// stops moving:
//
// 1. Sample an affine model of the objective and every constraint cell around
//    the current point by nudging each variable a little (a finite difference).
//    This gives the objective gradient and a linear approximation of every
//    constraint.
//
// 2. Build a quadratic model of the objective: its gradient plus a curvature
//    term held in a matrix that a damped BFGS (Broyden-Fletcher-Goldfarb-Shanno)
//    update keeps positive definite as the gradient changes from step to step.
//
// 3. Minimize that quadratic over the linearized constraints, kept as hard
//    constraints so every point stays feasible, inside a trust region. The
//    quadratic subproblem is solved by Frank-Wolfe: each inner iteration
//    minimizes a linear objective over the same feasible region with CoinMP and
//    takes a line-search step towards the vertex it returns.
//
// 4. Score the trial point with a merit function, its objective plus a large
//    penalty for any constraint violation, and keep it only if the merit beats
//    the current point. Resize the trust region from how well the quadratic
//    model predicted the real change, and refresh the curvature from the new
//    gradient.
//
// The loop ends when the step becomes negligible, the trust region collapses,
// the iteration limit is reached, or the time budget runs out. Success is
// reported only when the final point actually satisfies the constraints.
//
// The pieces are standard. The sequential quadratic model, the damped BFGS
// curvature update, the trust region, and the penalty merit function are
// covered by Nocedal and Wright, Numerical Optimization, second edition
// (Springer, 2006). Frank-Wolfe, used for the inner subproblem, is from Frank
// and Wolfe, "An algorithm for quadratic programming", Naval Research Logistics
// Quarterly 3 (1956), 95-110.
class SLPSolver : public SolverComponent
{
public:
    SLPSolver() {}

private:
    virtual void SAL_CALL solve() override;
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.Calc.SLPSolver"_ustr;
    }
    virtual OUString SAL_CALL getComponentDescription() override
    {
        return SolverComponent::GetResourceString(RID_SLP_SOLVER_COMPONENT);
    }

    void applyPoint(const std::vector<double>& rPoint);
    // Raw objective value at the current cell values.
    double objectiveAt();
    // Total amount by which the current cell values break the constraints.
    double violationAt();
    // Value to minimize: the objective for a minimize model, its negation for a
    // maximize model.
    double cost(double fObjective) const { return mbMaximize ? -fObjective : fObjective; }
    // Merit score of a point: its cost plus fPenalty for each unit of constraint
    // violation. Leaves the variable cells holding rPoint.
    double meritAt(const std::vector<double>& rPoint, double fPenalty);

    // Fill rLower and rUpper from the single-variable constant constraints and
    // the non-negative option.
    void deriveBounds(std::vector<double>& rLower, std::vector<double>& rUpper);
    // A representative magnitude for each variable: the width of a finite range,
    // otherwise the size of the starting value.
    std::vector<double> deriveScale(const std::vector<double>& rLower,
                                    const std::vector<double>& rUpper,
                                    const std::vector<double>& rStart);
    // The finite-difference step for each variable: a small fraction of its
    // scale, with an absolute floor so a near-zero scale is still probed.
    std::vector<double> deriveStep(const std::vector<double>& rScale);
    // The starting BFGS curvature matrix, a diagonal that balances the variable
    // scales so the quadratic term is comparable across variables.
    std::vector<double> initialCurvature(const std::vector<double>& rScale);

    // Build an affine model of the objective and every constraint cell around
    // rPoint. For each dependent cell the returned vector holds the model's value
    // extrapolated to the origin (all variables zero) at index 0, and the slope
    // along variable j from a finite difference of size rStep[j] at index j+1.
    // Leaves the variable cells holding rPoint.
    ScSolverCellHashMap sampleAffineModel(const std::vector<double>& rPoint,
                                          const std::vector<double>& rStep);

    // The gradient of the cost with respect to each variable, read from the
    // objective slopes in rCells. For a maximize model the sign is flipped so
    // the gradient always points in the minimize direction.
    std::vector<double> costGradient(const ScSolverCellHashMap& rCells);

    // Find the step d that most improves the quadratic model of the objective
    // without leaving the feasible region. The model is 0.5 d' B d + g' d, where
    // g (rGradient) is the objective gradient at the current point and B (rB) is
    // the BFGS curvature matrix, so the expression estimates how much the
    // objective changes for a step d. The feasible region is the linearized
    // constraints carried in rCells together with the trust-region box
    // [rTrustLower, rTrustUpper], so the new point rX + d obeys the linearized
    // constraints and stays inside the region where the model is trusted.
    //
    // The subproblem is solved by Frank-Wolfe (the conditional-gradient method),
    // which only ever minimizes a linear objective over that region, so each
    // inner step is a single CoinMP solve. An inner step takes the gradient of
    // the quadratic model at the current d (which is B d + g), asks CoinMP for
    // the feasible point that minimizes it (a vertex of the region), and moves d
    // partway towards that vertex by an exact line search of the quadratic. Every
    // vertex is feasible, so d stays feasible throughout, and the loop stops once
    // moving towards the best vertex can no longer improve the model.
    //
    // Returns the step d (the new point is rX + d), or an empty vector if the
    // linearized problem has no feasible point inside this trust region.
    std::vector<double> solveTrustRegionQp(const ScSolverCellHashMap& rCells,
                                           const std::vector<double>& rB,
                                           const std::vector<double>& rGradient,
                                           const std::vector<double>& rX,
                                           const std::vector<double>& rTrustLower,
                                           const std::vector<double>& rTrustUpper,
                                           const std::vector<char>& rColumnType, double fTimeout);

    // Write rPoint back as the solution when it satisfies the model, otherwise
    // record an infeasible status.
    void reportSolution(const std::vector<double>& rPoint);
};

double dotProduct(const std::vector<double>& rLeft, const std::vector<double>& rRight)
{
    double fSum = 0.0;
    for (size_t i = 0; i < rLeft.size(); ++i)
        fSum += rLeft[i] * rRight[i];
    return fSum;
}

// Dense row-major square matrix times a vector.
std::vector<double> multiplyMatrixVector(const std::vector<double>& rMatrix,
                                         const std::vector<double>& rVector)
{
    size_t nSize = rVector.size();
    std::vector<double> aResult(nSize, 0.0);
    for (size_t i = 0; i < nSize; ++i)
    {
        const double* pRow = &rMatrix[i * nSize];
        double fSum = 0.0;
        for (size_t j = 0; j < nSize; ++j)
            fSum += pRow[j] * rVector[j];
        aResult[i] = fSum;
    }
    return aResult;
}

// Refresh the curvature matrix rB from one step of progress, so the next
// quadratic model bends the way the objective actually bent over that step.
//
// BFGS (Broyden, Fletcher, Goldfarb and Shanno) learns the curvature rather
// than computing it. The true second derivatives would cost many extra cell
// recalculations, so instead it watches how the gradient changed
// (rGradientChange) over how far the point moved (rPointChange). A gradient
// that swings sharply over a short move means high curvature there. Each such
// pair folds into rB with a small update, so rB grows more accurate the longer
// the solver runs, at no extra sampling cost.
//
// The plain update only keeps rB a true upward-curving bowl (positive definite,
// the shape a minimum sits in) when the point move and the gradient change
// point the same way. The damping, due to Powell, blends the gradient change
// towards the old curvature whenever they do not, so rB stays positive definite
// even on a bumpy nonlinear problem.
void updateCurvature(std::vector<double>& rB, const std::vector<double>& rPointChange,
                     const std::vector<double>& rGradientChange)
{
    const size_t nVariables = rPointChange.size();
    std::vector<double> aBs = multiplyMatrixVector(rB, rPointChange);
    double fSBs = dotProduct(rPointChange, aBs);
    double fSy = dotProduct(rPointChange, rGradientChange);
    if (fSBs <= 0.0)
        return;

    constexpr double fDampingThreshold = 0.2;
    constexpr double fDampingScale = 0.8;
    double fTheta = 1.0;
    if (fSy < fDampingThreshold * fSBs)
        fTheta = fDampingScale * fSBs / (fSBs - fSy);
    std::vector<double> aR(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
    {
        aR[j] = fTheta * rGradientChange[j] + (1.0 - fTheta) * aBs[j];
    }
    double fSr = dotProduct(rPointChange, aR);
    if (fSr > 1.0e-12)
    {
        for (size_t i = 0; i < nVariables; ++i)
        {
            for (size_t j = 0; j < nVariables; ++j)
            {
                rB[i * nVariables + j] += -aBs[i] * aBs[j] / fSBs + aR[i] * aR[j] / fSr;
            }
        }
    }
}

void SLPSolver::applyPoint(const std::vector<double>& rPoint)
{
    for (size_t i = 0; i < maVariables.size(); ++i)
        SetValue(maVariables[i], rPoint[i]);
}

double SLPSolver::objectiveAt() { return GetValue(maObjective); }

double SLPSolver::violationAt()
{
    double fTotal = 0.0;
    for (const auto& rConstraint : maConstraints)
    {
        sheet::SolverConstraintOperator eOperator = rConstraint.Operator;
        if (eOperator != sheet::SolverConstraintOperator_LESS_EQUAL
            && eOperator != sheet::SolverConstraintOperator_GREATER_EQUAL
            && eOperator != sheet::SolverConstraintOperator_EQUAL)
            continue;

        double fLeft = GetValue(rConstraint.Left);
        double fRight = 0.0;
        table::CellAddress aRightAddress;
        if (rConstraint.Right >>= aRightAddress)
            fRight = GetValue(aRightAddress);
        else if (!(rConstraint.Right >>= fRight))
            continue;

        double fExcess;
        if (eOperator == sheet::SolverConstraintOperator_LESS_EQUAL)
            fExcess = fLeft - fRight; // Feasible when not positive.
        else if (eOperator == sheet::SolverConstraintOperator_GREATER_EQUAL)
            fExcess = fRight - fLeft;
        else
            fExcess = std::abs(fLeft - fRight); // Equality: any difference is a violation.

        if (fExcess > 0.0)
            fTotal += fExcess;
    }
    return fTotal;
}

double SLPSolver::meritAt(const std::vector<double>& rPoint, double fPenalty)
{
    applyPoint(rPoint);
    return cost(objectiveAt()) + fPenalty * violationAt();
}

void SLPSolver::deriveBounds(std::vector<double>& rLower, std::vector<double>& rUpper)
{
    const size_t nVariables = maVariables.size();
    // Fold single-variable constant constraints into box bounds. They remain
    // ordinary constraint rows too.
    rLower.assign(nVariables, mbNonNegative ? 0.0 : -DBL_MAX);
    rUpper.assign(nVariables, DBL_MAX);

    // A constant on the right and the variable on the left is a plain bound. It
    // is still passed to the subproblem as an ordinary row as well.
    for (const auto& rConstraint : maConstraints)
    {
        sheet::SolverConstraintOperator eOperator = rConstraint.Operator;
        double fRhs = 0.0;
        if (!(rConstraint.Right >>= fRhs))
            continue;
        for (size_t j = 0; j < nVariables; ++j)
        {
            if (!AddressEqual(maVariables[j], rConstraint.Left))
                continue;
            if (eOperator == sheet::SolverConstraintOperator_LESS_EQUAL && fRhs < rUpper[j])
                rUpper[j] = fRhs;
            else if (eOperator == sheet::SolverConstraintOperator_GREATER_EQUAL && fRhs > rLower[j])
                rLower[j] = fRhs;
            else if (eOperator == sheet::SolverConstraintOperator_EQUAL)
            {
                rLower[j] = fRhs;
                rUpper[j] = fRhs;
            }
        }
    }
}

std::vector<double> SLPSolver::deriveScale(const std::vector<double>& rLower,
                                           const std::vector<double>& rUpper,
                                           const std::vector<double>& rStart)
{
    const size_t nVariables = maVariables.size();
    std::vector<double> aScale(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
    {
        // The width of a finite range, otherwise the size of the starting value.
        if (rLower[j] > -DBL_MAX && rUpper[j] < DBL_MAX && rUpper[j] > rLower[j])
            aScale[j] = rUpper[j] - rLower[j];
        else
            aScale[j] = std::max(1.0, std::abs(rStart[j]));
    }
    return aScale;
}

std::vector<double> SLPSolver::deriveStep(const std::vector<double>& rScale)
{
    const size_t nVariables = maVariables.size();
    constexpr double fRelativeStep = 1.0e-6;
    constexpr double fStepFloor = 1.0e-7;
    std::vector<double> aStep(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
        aStep[j] = std::max(fStepFloor, fRelativeStep * rScale[j]);
    return aStep;
}

std::vector<double> SLPSolver::initialCurvature(const std::vector<double>& rScale)
{
    const size_t nVariables = maVariables.size();
    std::vector<double> aB(nVariables * nVariables, 0.0);
    for (size_t j = 0; j < nVariables; ++j)
        aB[j * nVariables + j] = 1.0 / (rScale[j] * rScale[j]);
    return aB;
}

std::vector<double>
SLPSolver::solveTrustRegionQp(const ScSolverCellHashMap& rCells, const std::vector<double>& rB,
                              const std::vector<double>& rGradient, const std::vector<double>& rX,
                              const std::vector<double>& rTrustLower,
                              const std::vector<double>& rTrustUpper,
                              const std::vector<char>& rColumnType, double fTimeout)
{
    const size_t nVariables = rX.size();
    std::vector<double> aD(nVariables, 0.0); // Step, starts at zero (the point rX).

    // The linearized constraints stay the same throughout. Only the objective
    // row changes each iteration, so copy the cells once and rewrite that row in
    // place. The map is never restructured here, so the reference stays valid.
    ScSolverCellHashMap aLinearCells = rCells;
    std::vector<double>& rObjectCoefficients = aLinearCells[maObjective];

    constexpr int nMaxFrankWolfe = 50;
    constexpr double fGapTolerance = 1.0e-9;
    constexpr double fGapFraction = 1.0e-3;
    double fInitialGap = 0.0;
    for (int nFrankWolfe = 0; nFrankWolfe < nMaxFrankWolfe; ++nFrankWolfe)
    {
        // Gradient of the quadratic model at the current step: B d + g.
        std::vector<double> aQuadraticGradient = multiplyMatrixVector(rB, aD);
        for (size_t j = 0; j < nVariables; ++j)
            aQuadraticGradient[j] += rGradient[j];

        // Minimize that linear objective over the feasible region: put the
        // quadratic-model gradient in as the objective coefficients.
        rObjectCoefficients.assign(nVariables + 1, 0.0);
        for (size_t j = 0; j < nVariables; ++j)
            rObjectCoefficients[j + 1] = aQuadraticGradient[j];

        CoinMpSolveResult aLinearResult = coinmpSolveLinearModel(
            maVariables, maConstraints, maObjective, aLinearCells, rTrustLower, rTrustUpper,
            rColumnType, /*bMaximize*/ false, fTimeout);
        if (!aLinearResult.bSuccess)
            return std::vector<double>(); // Infeasible in this trust region.

        // Frank-Wolfe vertex as a step from rX, and the search direction.
        std::vector<double> aDirection(nVariables);
        for (size_t j = 0; j < nVariables; ++j)
            aDirection[j] = (aLinearResult.aSolution[j] - rX[j]) - aD[j];

        // Duality gap: how much the linear model could still improve.
        double fGap = -dotProduct(aQuadraticGradient, aDirection);
        if (nFrankWolfe == 0)
            fInitialGap = fGap;
        // Stop once the subproblem is solved well enough: the gap has shrunk to
        // a small fraction of its initial value, or below an absolute floor.
        // Driving the gap all the way to zero is wasted work, because the outer
        // trust-region loop re-samples and re-solves around the accepted step.
        if (fGap < fGapTolerance || fGap < fGapFraction * fInitialGap)
            break;

        // Exact line search of the quadratic along the Frank-Wolfe direction.
        std::vector<double> aBDirection = multiplyMatrixVector(rB, aDirection);
        double fDenominator = dotProduct(aDirection, aBDirection);
        double fGamma = fDenominator > 1.0e-12 ? std::clamp(fGap / fDenominator, 0.0, 1.0) : 1.0;

        for (size_t j = 0; j < nVariables; ++j)
            aD[j] += fGamma * aDirection[j];
    }
    return aD;
}

ScSolverCellHashMap SLPSolver::sampleAffineModel(const std::vector<double>& rPoint,
                                                 const std::vector<double>& rStep)
{
    const size_t nVariables = maVariables.size();

    // The dependent cells are the objective and each side of every constraint.
    ScSolverCellHashMap aCells;
    aCells[maObjective].reserve(nVariables + 1);
    for (const auto& rConstraint : maConstraints)
    {
        aCells[rConstraint.Left].reserve(nVariables + 1);
        table::CellAddress aRightAddress;
        if (rConstraint.Right >>= aRightAddress)
            aCells[aRightAddress].reserve(nVariables + 1);
    }

    // Value of every dependent cell at the base point, stored at index 0 for now.
    for (size_t j = 0; j < nVariables; ++j)
        SetValue(maVariables[j], rPoint[j]);
    for (auto& rEntry : aCells)
        rEntry.second.push_back(GetValue(rEntry.first));

    // Perturb one variable at a time to estimate the slope of every cell.
    for (size_t j = 0; j < nVariables; ++j)
    {
        SetValue(maVariables[j], rPoint[j] + rStep[j]);
        for (auto& rEntry : aCells)
        {
            double fBase = rEntry.second.front();
            double fPerturbed = GetValue(rEntry.first);
            rEntry.second.push_back((fPerturbed - fBase) / rStep[j]);
        }
        SetValue(maVariables[j], rPoint[j]);
    }

    // Turn the base value at index 0 into the model's value at the origin:
    // f(0) = f(point) - sum_j (slope_j * point_j).
    for (auto& rEntry : aCells)
    {
        double fAtOrigin = rEntry.second.front();
        for (size_t j = 0; j < nVariables; ++j)
            fAtOrigin -= rEntry.second[j + 1] * rPoint[j];
        rEntry.second.front() = fAtOrigin;
    }

    return aCells;
}

std::vector<double> SLPSolver::costGradient(const ScSolverCellHashMap& rCells)
{
    const size_t nVariables = maVariables.size();
    const std::vector<double>& rObjectCoefficients = rCells.at(maObjective);
    double fSign = mbMaximize ? -1.0 : 1.0;
    std::vector<double> aGradient(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
        aGradient[j] = fSign * rObjectCoefficients[j + 1];
    return aGradient;
}

void SLPSolver::reportSolution(const std::vector<double>& rPoint)
{
    applyPoint(rPoint);
    double fObjective = objectiveAt();
    double fViolation = violationAt();

    // Report success only when the final point really satisfies the model.
    // The tolerance scales with the objective magnitude.
    constexpr double fFeasibilityTolerance = 1.0e-6;
    bool bFeasible = fViolation <= fFeasibilityTolerance * (1.0 + std::abs(fObjective));
    mbSuccess = bFeasible;
    if (bFeasible)
    {
        maSolution.realloc(maVariables.size());
        std::copy(rPoint.begin(), rPoint.end(), maSolution.getArray());
        mfResultValue = fObjective;
    }
    else
    {
        maStatus = SolverComponent::GetResourceString(RID_ERROR_INFEASIBLE);
    }
}

void SAL_CALL SLPSolver::solve()
{
    uno::Reference<frame::XModel> xModel(mxDoc, uno::UNO_QUERY_THROW);

    maStatus.clear();
    mbSuccess = false;

    const size_t nVariables = maVariables.size();
    if (nVariables == 0)
        return;

    xModel->lockControllers();
    comphelper::ScopeGuard aUnlockGuard([&xModel] { xModel->unlockControllers(); });

    std::vector<double> aLower;
    std::vector<double> aUpper;
    deriveBounds(aLower, aUpper);

    // Starting point: the current cell values, clamped to the bounds.
    std::vector<double> aX(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
        aX[j] = std::clamp(GetValue(maVariables[j]), aLower[j], aUpper[j]);

    std::vector<double> aScale = deriveScale(aLower, aUpper, aX);
    std::vector<double> aStep = deriveStep(aScale);

    std::vector<char> aColumnType(nVariables, mbInteger ? 'I' : 'C');

    std::vector<double> aB = initialCurvature(aScale);

    // SQP tuning parameters. The trust region is a fraction of each variable's
    // scale: it grows by fTrustGrow when the quadratic model predicts a step well
    // (ratio above fGoodRatio) and shrinks by fTrustShrink otherwise (ratio below
    // fPoorRatio, an empty subproblem, or a rejected step). The loop stops once
    // the relative step is below fStepTolerance or the radius below fTrustMin.
    constexpr double fPenalty = 1.0e4;
    constexpr double fInitialTrust = 0.3;
    constexpr double fTrustMin = 1.0e-9;
    constexpr double fTrustMax = 1.0;
    constexpr double fTrustGrow = 2.0;
    constexpr double fTrustShrink = 0.5;
    constexpr double fGoodRatio = 0.75;
    constexpr double fPoorRatio = 0.25;
    constexpr double fStepTolerance = 1.0e-8;
    constexpr double fMinPredictedReduction = 1.0e-12;
    constexpr double fDefaultTimeout = 100.0;
    // High safety limit only. The time budget and the convergence test below
    // are what normally end the loop, so a large model is not cut off early.
    constexpr sal_Int32 nMaxIterations = 100000;

    double fTrust = fInitialTrust;
    double fMerit = meritAt(aX, fPenalty);
    ScSolverCellHashMap aCells = sampleAffineModel(aX, aStep);
    std::vector<double> aGradient = costGradient(aCells);

    const double fTimeout = mnTimeout > 0 ? double(mnTimeout) : fDefaultTimeout;
    auto aStart = std::chrono::steady_clock::now();

    for (sal_Int32 nIteration = 0; nIteration < nMaxIterations; ++nIteration)
    {
        std::chrono::duration<double> aElapsed = std::chrono::steady_clock::now() - aStart;
        if (aElapsed.count() >= fTimeout)
            break;

        // Trust-region bounds, kept inside the real variable bounds.
        std::vector<double> aTrustLower(nVariables);
        std::vector<double> aTrustUpper(nVariables);
        for (size_t j = 0; j < nVariables; ++j)
        {
            aTrustLower[j] = std::max(aLower[j], aX[j] - fTrust * aScale[j]);
            aTrustUpper[j] = std::min(aUpper[j], aX[j] + fTrust * aScale[j]);
            if (aTrustUpper[j] < aTrustLower[j])
                aTrustUpper[j] = aTrustLower[j];
        }

        std::vector<double> aDelta = solveTrustRegionQp(aCells, aB, aGradient, aX, aTrustLower,
                                                        aTrustUpper, aColumnType, fTimeout);
        if (aDelta.empty())
        {
            fTrust *= fTrustShrink;
            if (fTrust < fTrustMin)
                break;
            continue;
        }

        std::vector<double> aTrial(nVariables);
        for (size_t j = 0; j < nVariables; ++j)
            aTrial[j] = aX[j] + aDelta[j];

        double fMeritTrial = meritAt(aTrial, fPenalty);

        // Reduction the quadratic model predicted: -(g'd + 0.5 d'B d).
        std::vector<double> aBDelta = multiplyMatrixVector(aB, aDelta);
        double fPredicted = -(dotProduct(aGradient, aDelta) + 0.5 * dotProduct(aDelta, aBDelta));
        double fActual = fMerit - fMeritTrial;

        double fStepRelative = 0.0;
        for (size_t j = 0; j < nVariables; ++j)
            fStepRelative = std::max(fStepRelative, std::abs(aDelta[j]) / aScale[j]);

        if (fActual > 0.0)
        {
            // Accept: sample at the new point and update the curvature.
            ScSolverCellHashMap aCellsNew = sampleAffineModel(aTrial, aStep);
            std::vector<double> aNewGradient = costGradient(aCellsNew);

            std::vector<double> aPointChange(nVariables);
            std::vector<double> aGradientChange(nVariables);
            for (size_t j = 0; j < nVariables; ++j)
            {
                aPointChange[j] = aTrial[j] - aX[j];
                aGradientChange[j] = aNewGradient[j] - aGradient[j];
            }
            updateCurvature(aB, aPointChange, aGradientChange);

            aX = aTrial;
            aGradient = aNewGradient;
            aCells = aCellsNew;
            fMerit = fMeritTrial;

            double fRatio = fPredicted > fMinPredictedReduction ? fActual / fPredicted : 1.0;
            if (fRatio > fGoodRatio)
                fTrust = std::min(fTrustMax, fTrust * fTrustGrow);
            else if (fRatio < fPoorRatio)
                fTrust *= fTrustShrink;
        }
        else
        {
            // The trial did not improve the merit, so reject it and shrink the
            // trust region.
            fTrust *= fTrustShrink;
        }

        if (fStepRelative < fStepTolerance && fActual <= 0.0)
            break;
        if (fTrust < fTrustMin)
            break;
    }

    reportSolution(aX);
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Calc_SLPSolver_get_implementation(css::uno::XComponentContext*,
                                                    css::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new SLPSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
