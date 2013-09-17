/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"
#include "scmatrix.hxx"

#include "openclwrapper.hxx"

#define USE_GROUNDWATER_INTERPRETER 0

#define SRCDATASIZE 100
#define SINGLEARRAYLEN 100
#define DOUBLEARRAYLEN 100
#define SVDOUBLELEN 100

namespace sc { namespace opencl {

/////time test dbg
double getTimeDiff(const TimeValue& t1, const TimeValue& t2)
{
    double tv1 = t1.Seconds;
    double tv2 = t2.Seconds;
    tv1 += t1.Nanosec / 1000000000.0;
    tv2 += t2.Nanosec / 1000000000.0;

    return tv1 - tv2;
}//dbg-t
TimeValue aTimeBefore, aTimeAfter;
///////////////////////////////////////
class SourceData
{
    const double *mdpSrcPtr;
    unsigned int mnDataSize;
    const char *mcpFormulaName;
    unsigned int mnCol;
    int eOp;
public:
    SourceData( const double *dpData, unsigned int nSize, uint nCol = 1,const char *cpFormulaName = NULL):mdpSrcPtr(dpData),mnDataSize(nSize),mcpFormulaName(cpFormulaName),mnCol(nCol)
    {
    }
    SourceData():mdpSrcPtr(NULL),mnDataSize(0)
    {
    }
    void setSrcPtr( const double *dpTmpDataPtr)
    {
        mdpSrcPtr = dpTmpDataPtr;
    }
    void setSrcSize( int nSize )
    {
        mnDataSize = nSize;
    }
    const double * getDouleData()
    {
        return mdpSrcPtr;
    }
    unsigned int getDataSize()
    {
        return mnDataSize;
    }
    void print()
    {
        for( uint i=0; i<mnDataSize; i++ )
            printf( " The SourceData is %f and data size is %d\n",mdpSrcPtr[i],mnDataSize );
    }
    void printFormula()
    {
        printf("--------The formulaname is %s and the eOp is %d---------\n",mcpFormulaName,eOp);
    }
    void setFormulaName(const char *cpFormulaName)
    {
        this->mcpFormulaName = cpFormulaName;
    }
    const char *getFormulaName()
    {
        return mcpFormulaName;
    }
    void seteOp(int op)
    {
        this->eOp = op;
    }
    int geteOp()
    {
        return eOp;
    }
    int getColNum()
    {
        return mnCol;
    }

};

class FormulaGroupInterpreterOpenCL : public FormulaGroupInterpreter
{
    SourceData *mSrcDataStack[SRCDATASIZE];
    unsigned int mnStackPointer,mnDoublePtrCount;
    uint * mnpOclStartPos;
    uint * mnpOclEndPos;
    SingleVectorFormula *mSingleArray[SINGLEARRAYLEN];
    DoubleVectorFormula *mDoubleArray[DOUBLEARRAYLEN];
    double mdpSvdouble[SVDOUBLELEN];
    double *mdpSrcDoublePtr[SVDOUBLELEN];
    uint mnSingleCount;
    uint mnDoubleCount;
    uint mnSvDoubleCount;
    uint mnOperatorGroup[100];
    uint mnOperatorCount;
    char mcHostName[100];
    uint mnPositonLen;
    size_t mnRowSize;
public:
    FormulaGroupInterpreterOpenCL() :
        FormulaGroupInterpreter()
    {
        mnStackPointer = 0;
        mnpOclEndPos = NULL;
        mnpOclStartPos = NULL;
        mnSingleCount = 0;
        mnDoubleCount = 0;
        mnSvDoubleCount = 0;
        mnOperatorCount = 0;
        mnPositonLen = 0;
        mnDoublePtrCount = 0;
    }
    virtual ~FormulaGroupInterpreterOpenCL()
    {
    }

    virtual ScMatrixRef inverseMatrix( const ScMatrix& rMat );
    virtual bool interpret( ScDocument& rDoc, const ScAddress& rTopPos,
                           const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode );
    void collectDoublePointers( double *temp )
    {
        if( mnDoublePtrCount < SRCDATASIZE )
        {
            mdpSrcDoublePtr[mnDoublePtrCount++] = temp;
        }
        else
        {
            printf( "The mdpSrcDoublePtr is full now.\n" );
            double *dtmp = NULL;
            if ( (dtmp = mdpSrcDoublePtr[--mnDoublePtrCount]) != NULL )
            {
                free( dtmp );
                dtmp = NULL;
            }
        }
    }

    void freeDoublePointers()
    {
        while( mnDoublePtrCount > 0 )
        {
            double *dtmp = NULL;
            if ( (dtmp = mdpSrcDoublePtr[--mnDoublePtrCount]) != NULL )
            {
                free( dtmp );
                dtmp = NULL;
            }
        }
    }


    void srdDataPush( SourceData *temp )
    {
        if( mnStackPointer < SRCDATASIZE )
        {
            mSrcDataStack[mnStackPointer++] = temp;
        }
        else
            printf( "The stack is full now.\n" );
    }
    SourceData *srdDataPop( void )
    {
        if( mnStackPointer <= 0 )
        {
            printf( "The stack was empty\n" );
            return NULL;
        }
        return mSrcDataStack[--mnStackPointer];
    }
    unsigned int getDataSize()
    {
        return mnStackPointer;
    }
    void printStackInfo()
    {
        printf( "/********The stack size is %d*********\\\n",mnStackPointer );
        for ( int i = mnStackPointer - 1; i >= 0; i-- )
            mSrcDataStack[i]->print();
    }
    bool getPosition(const ScTokenArray& rCode,const ScFormulaCellGroupRef& xGroup,uint nRowSize,uint *&npOclStartPos,uint *&npOclEndPos,uint *nPositonLen);
    bool chooseFunction(OclCalc &ocl_calc,double *&dpResult);
    bool isStockHistory();
    bool isGroundWater();
};
bool FormulaGroupInterpreterOpenCL::getPosition(const ScTokenArray& rCode,const ScFormulaCellGroupRef& xGroup,uint nRowSize,uint *&npOclStartPos,uint *&npOclEndPos,uint *nPositonLen)
{
        uint nColPosition = 0;
        ScTokenArray * rCodePos = rCode.Clone();
        static int nCountPosSize = nRowSize;
        bool isAllocFormulaOclBuf = true;
        for ( const formula::FormulaToken* p = rCodePos->First(); p; p = rCodePos->Next() )
        {
            if ( p->GetType() == formula::svDoubleVectorRef )
            {
                nColPosition++;
            }
        }
        int nPositionSize = nColPosition * nRowSize;
        npOclStartPos = (unsigned int*) malloc( nPositionSize * sizeof(unsigned int) );
        npOclEndPos = (unsigned int*) malloc( nPositionSize * sizeof(unsigned int) );
        if ( nCountPosSize < nPositionSize )
        {
            nCountPosSize = nPositionSize;
            isAllocFormulaOclBuf = false;
        }
        for ( sal_Int32 i = 0; i < xGroup->mnLength; ++i )
        {
            ScTokenArray * rCodeTemp = rCode.Clone();
            int j = 0;
            for ( const formula::FormulaToken* p = rCodeTemp->First(); p; p = rCodeTemp->Next() )
            {
                if (p->GetType() == formula::svDoubleVectorRef)
                {
                    const formula::DoubleVectorRefToken* p2 = static_cast<const formula::DoubleVectorRefToken*>(p);
                    size_t nRowStart = p2->IsStartFixed() ? 0 : i;
                    size_t nRowEnd = p2->GetRefRowSize() - 1;
                    if (!p2->IsEndFixed())
                        nRowEnd += i;
                    npOclStartPos[j*nRowSize+i] = nRowStart;//record the start position
                    npOclEndPos[j*nRowSize+i] = nRowEnd;//record the end position
                    j++;
                }
            }
        }
        *nPositonLen = nPositionSize;
        //Now the pos array is 0 1 2 3 4 5  0 1 2 3 4 5;
        return isAllocFormulaOclBuf;
}

bool FormulaGroupInterpreterOpenCL::isStockHistory()
{
    bool isHistory = false;
    if( (mnOperatorGroup[0]== 224) && (mnOperatorGroup[1]== 227) && (mnOperatorGroup[2]== 41) && (mnOperatorGroup[3]== 43) && (mnOperatorGroup[4]== 41) )
    {
        strcpy(mcHostName,"OclOperationColumnN");
        isHistory = true;
    }
    else if( (mnOperatorGroup[0] == 226) && (mnOperatorGroup[1] == 42) )
    {
        strcpy(mcHostName,"OclOperationColumnH");
        isHistory = true;
    }
    else if((mnOperatorGroup[0] == 213) && (mnOperatorGroup[1] == 43) && (mnOperatorGroup[2] == 42) )
    {
        strcpy(mcHostName,"OclOperationColumnJ");
        isHistory = true;
    }
    return isHistory;
}

bool FormulaGroupInterpreterOpenCL::isGroundWater()
{
    bool GroundWater=false;

    if((mnOperatorGroup[0] == ocAverage && 1 == mnSingleCount )||(mnOperatorGroup[0] == ocMax && 1 == mnSingleCount )||
        (mnOperatorGroup[0] == ocMin && 1 == mnSingleCount )||(mnOperatorGroup[0] == ocSub && mnSvDoubleCount==1))
    {
        GroundWater = true;
    }
    return GroundWater;
}

bool FormulaGroupInterpreterOpenCL::chooseFunction( OclCalc &ocl_calc, double *&dpResult )
{
    const double * dpOclSrcData = NULL;
    unsigned int nSrcDataSize = 0;
    const double *dpLeftData = NULL;
    const double *dpRightData = NULL;
    if((mnOperatorGroup[0] == ocAverage && 1 == mnSingleCount )||(mnOperatorGroup[0] == ocMax && 1 == mnSingleCount )||
        (mnOperatorGroup[0] == ocMin && 1 == mnSingleCount )||(mnOperatorGroup[0] == ocSub && mnSvDoubleCount==1))
    {
        double delta = 0.0;
        const double *pArrayToSubtractOneElementFrom;
        const double *pGroundWaterDataArray;
        uint nSrcData = 0;
        if( mnSvDoubleCount!=1 )
        {
            pArrayToSubtractOneElementFrom= mSingleArray[0]->mdpInputLeftData;
            pGroundWaterDataArray= mDoubleArray[0]->mdpInputData;
            nSrcData = mDoubleArray[0]->mnInputDataSize;
        }
        else
        {
            pArrayToSubtractOneElementFrom= mSingleArray[0]->mdpInputLeftData;
            pGroundWaterDataArray=NULL;
            delta = mdpSvdouble[0];
        }

        bool bSuccess = ocl_calc.oclGroundWaterGroup(
            mnOperatorGroup, mnOperatorCount, pGroundWaterDataArray,
            pArrayToSubtractOneElementFrom, nSrcData, mnRowSize, delta,
            mnpOclStartPos, mnpOclEndPos, dpResult);

        if (!bSuccess)
            return false;
    }
    else if( isStockHistory() )
    {
        return false;
    }
    else if(((mnSvDoubleCount==0)&&(mnSingleCount==0)&&(mnDoubleCount==1)) &&
            ((mnOperatorGroup[0] == ocAverage)||(mnOperatorGroup[0] == ocMax)||(mnOperatorGroup[0] == ocMin)))
    {
        if(mnOperatorGroup[0] == ocAverage)
            strcpy(mcHostName,"oclFormulaAverage");
        if(mnOperatorGroup[0] == ocMax)
            strcpy(mcHostName,"oclFormulaMax");
        if(mnOperatorGroup[0] == ocMin)
            strcpy(mcHostName,"oclFormulaMin");
        DoubleVectorFormula * doubleTemp = mDoubleArray[--mnDoubleCount];
        nSrcDataSize = doubleTemp->mnInputDataSize;
        dpOclSrcData = doubleTemp->mdpInputData;
        if ( ocl_calc.getOpenclState())
        {
            if ( ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag == 1 )
            {
                if (!ocl_calc.createFormulaBuf64Bits(nSrcDataSize, mnRowSize))
                    return false;
                if (!ocl_calc.mapAndCopy64Bits(dpOclSrcData, mnpOclStartPos, mnpOclEndPos, nSrcDataSize, mnRowSize))
                    return false;
                if (!ocl_calc.oclHostFormulaStatistics64Bits(mcHostName, dpResult, mnRowSize))
                    return false;
            }
            else
            {
                if (!ocl_calc.createFormulaBuf32Bits(nSrcDataSize, mnPositonLen))
                    return false;
                if (!ocl_calc.mapAndCopy32Bits(dpOclSrcData, mnpOclStartPos, mnpOclEndPos, nSrcDataSize, mnRowSize))
                    return false;
                if (!ocl_calc.oclHostFormulaStatistics32Bits(mcHostName, dpResult, mnRowSize))
                    return false;
            }
        }
    }
    else if((mnSvDoubleCount==0)&&(mnSingleCount==1)&&(mnDoubleCount==0))
    {
        dpLeftData = mSingleArray[0]->mdpInputLeftData;
        dpRightData =  mSingleArray[0]->mdpInputRightData;
        if(mnOperatorGroup[0] == ocAdd)
            strcpy(mcHostName,"oclSignedAdd");
        if(mnOperatorGroup[0] == ocSub)
            strcpy(mcHostName,"oclSignedSub");
        if(mnOperatorGroup[0] == ocMul)
            strcpy(mcHostName,"oclSignedMul");
        if(mnOperatorGroup[0] == ocDiv)
            strcpy(mcHostName,"oclSignedDiv");
        if ( ocl_calc.getOpenclState())
        {
            if ( ocl_calc.gpuEnv.mnKhrFp64Flag == 1 || ocl_calc.gpuEnv.mnAmdFp64Flag == 1 )
            {
                if (!ocl_calc.createArithmeticOptBuf64Bits(mnRowSize))
                    return false;
                if (!ocl_calc.mapAndCopy64Bits(dpLeftData, dpRightData, mnRowSize))
                    return false;
                if (!ocl_calc.oclHostArithmeticOperator64Bits(mcHostName, dpResult, mnRowSize))
                    return false;
            }
            else
            {
                if (!ocl_calc.createArithmeticOptBuf32Bits(mnRowSize))
                    return false;
                if (!ocl_calc.mapAndCopy32Bits(dpLeftData, dpRightData, mnRowSize))
                    return false;
                if (!ocl_calc.oclHostArithmeticOperator32Bits(mcHostName, dpResult, mnRowSize))
                    return false;
            }
        }
    }
    else if( (mnSingleCount>1) && (mnSvDoubleCount==0) && (mnDoubleCount==0) )
    {
        const double* dpArray[100] = {};
        int j=0;
        for( uint i = 0; i < mnSingleCount; i++ )
        {
            dpArray[j++] = mSingleArray[i]->mdpInputLeftData;
            if( NULL != mSingleArray[i]->mdpInputRightData )
                dpArray[j++] = mSingleArray[i]->mdpInputRightData;
        }
        double *dpMoreColArithmetic = (double *)malloc( sizeof(double) * j * mnRowSize );
        if( NULL == dpMoreColArithmetic )
        {
            printf("Memory alloc error!\n");
            return false;
        }
        for( uint i = 0; i < j*mnRowSize; i++ )
        {
            dpMoreColArithmetic[i] = dpArray[i/mnRowSize][i%mnRowSize];
        }
        if ( ocl_calc.getOpenclState())
        {
            if ( ocl_calc.gpuEnv.mnKhrFp64Flag == 1 || ocl_calc.gpuEnv.mnAmdFp64Flag == 1 )
            {
                if (!ocl_calc.createMoreColArithmeticBuf64Bits(j * mnRowSize, mnOperatorCount))
                    return false;
                if (!ocl_calc.mapAndCopyMoreColArithmetic64Bits(dpMoreColArithmetic, mnRowSize * j, mnOperatorGroup, mnOperatorCount))
                    return false;
                if (!ocl_calc.oclMoreColHostArithmeticOperator64Bits(mnRowSize, mnOperatorCount, dpResult, mnRowSize))
                    return false;
            }
            else
            {
                if (!ocl_calc.createMoreColArithmeticBuf32Bits(j* mnRowSize, mnOperatorCount))
                    return false;
                if (!ocl_calc.mapAndCopyMoreColArithmetic32Bits(dpMoreColArithmetic, mnRowSize * j, mnOperatorGroup, mnOperatorCount))
                    return false;
                if (!ocl_calc.oclMoreColHostArithmeticOperator32Bits(mnRowSize, mnOperatorCount, dpResult, mnRowSize))
                    return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

class agency
{
public:
    double *calculate(int nOclOp,int rowSize,OclCalc &ocl_calc,uint *npOclStartPos,uint *npOclEndPos,FormulaGroupInterpreterOpenCL *formulaInterprt);
};

double * agency::calculate( int nOclOp,int rowSize,OclCalc &ocl_calc,uint *npOclStartPos,uint *npOclEndPos,FormulaGroupInterpreterOpenCL *formulaInterprt)
{
    const double *dpLeftData = NULL;
    const double *dpRightData = NULL;
    const double *dpOclSrcData=NULL;
    if ( ocl_calc.gpuEnv.mnKhrFp64Flag == 1 || ocl_calc.gpuEnv.mnAmdFp64Flag == 1 )
    {
        switch( nOclOp )
        {
            case ocAdd:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset(rResult,0,rowSize);
                ocl_calc.oclHostArithmeticStash64Bits( "oclSignedAdd",dpLeftData,dpRightData,rResult,temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            case ocSub:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = ( double * )malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostArithmeticStash64Bits( "oclSignedSub",dpLeftData,dpRightData,rResult,temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData(rResult,rowSize) );
                break;
            }
            case ocMul:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostArithmeticStash64Bits( "oclSignedMul",dpLeftData,dpRightData,rResult,temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            case ocDiv:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = ( double * )malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostArithmeticStash64Bits( "oclSignedDiv",dpLeftData,dpRightData,rResult,temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            case ocMax:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostFormulaStash64Bits( "oclFormulaMax",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,nDataSize,rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            case ocMin:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostFormulaStash64Bits( "oclFormulaMin",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,nDataSize,rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            case ocAverage:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult,0,rowSize );
                ocl_calc.oclHostFormulaStash64Bits( "oclFormulaAverage",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,nDataSize,rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult,rowSize ) );
                break;
            }
            default:
                fprintf( stderr,"No OpenCL function for this calculation.\n" );
                break;
        }
    }
    else
    {
        switch( nOclOp )
        {
            case ocAdd:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset(rResult,0,rowSize);
                ocl_calc.oclHostArithmeticStash32Bits( "oclSignedAdd", dpLeftData, dpRightData, rResult, temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData(rResult, rowSize) );
                break;
            }
            case ocSub:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult, 0, rowSize );
                ocl_calc.oclHostArithmeticStash32Bits( "oclSignedSub", dpLeftData, dpRightData, rResult, temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData( rResult, rowSize ) );
                break;
            }
            case ocMul:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc(sizeof(double) * rowSize );
                memset( rResult, 0, rowSize );
                ocl_calc.oclHostArithmeticStash32Bits( "oclSignedMul", dpLeftData, dpRightData, rResult, temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData( rResult, rowSize ) );
                break;
            }
            case ocDiv:
            {
                SourceData *temp = formulaInterprt->srdDataPop();
                SourceData *temp2 = formulaInterprt->srdDataPop();
                dpLeftData = temp2->getDouleData();
                dpRightData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult, 0, rowSize );
                ocl_calc.oclHostArithmeticStash32Bits( "oclSignedDiv", dpLeftData, dpRightData, rResult, temp->getDataSize() );
                formulaInterprt->srdDataPush( new SourceData(rResult, rowSize) );
                break;
            }
            case ocMax:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc(sizeof(double) * rowSize );
                memset(rResult,0,rowSize);
                ocl_calc.oclHostFormulaStash32Bits( "oclFormulaMax", dpOclSrcData, npOclStartPos, npOclEndPos, rResult,nDataSize, rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult, rowSize ) );
                break;
            }
            case ocMin:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult, 0, rowSize );
                ocl_calc.oclHostFormulaStash32Bits( "oclFormulaMin", dpOclSrcData, npOclStartPos, npOclEndPos, rResult, nDataSize, rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult, rowSize) );
                break;
            }
            case ocAverage:
            {
                unsigned int nDataSize = 0;
                SourceData *temp = formulaInterprt->srdDataPop();
                nDataSize = temp->getDataSize();
                dpOclSrcData = temp->getDouleData();
                double *rResult = NULL; // Point to the output data from GPU
                rResult = (double *)malloc( sizeof(double) * rowSize );
                memset( rResult, 0, rowSize);
                ocl_calc.oclHostFormulaStash32Bits( "oclFormulaAverage", dpOclSrcData, npOclStartPos, npOclEndPos, rResult, nDataSize, rowSize );
                formulaInterprt->srdDataPush( new SourceData( rResult, rowSize) );
                break;
            }
            default:
                fprintf(stderr,"No OpenCL function for this calculation.\n");
                break;
        }
    }
    return NULL;
}

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& rMat )
{
    SCSIZE nC, nR;
    rMat.GetDimensions( nC, nR );
    if ( nC != nR || nC == 0 )
        // Input matrix must be square. Return an empty matrix on failure and
        // the caller will calculate it via CPU.
        return ScMatrixRef();

    // This vector will contain a series of doubles from the first column to
    // the last, chained together in a single array.
    std::vector<double> aDoubles;
    rMat.GetDoubleArray(aDoubles);

    float * fpOclMatrixSrc = NULL;
    float * fpOclMatrixDst = NULL;
    double * dpOclMatrixSrc = NULL;
    double * dpOclMatrixDst = NULL;
    uint nMatrixSize = nC * nR;
    static OclCalc aOclCalc;
    if ( aOclCalc.getOpenclState() )
    {
        if ( aOclCalc.gpuEnv.mnKhrFp64Flag == 1 || aOclCalc.gpuEnv.mnAmdFp64Flag == 1 )
        {
            aOclCalc.createBuffer64Bits( dpOclMatrixSrc, dpOclMatrixDst, nMatrixSize );
            for ( uint i = 0; i < nC; i++ )
                for ( uint j = 0; j < nR; j++ )
                    dpOclMatrixSrc[i*nC+j] = aDoubles[j*nR+i];
            aOclCalc.oclHostMatrixInverse64Bits( "oclFormulaMtxInv", dpOclMatrixSrc, dpOclMatrixDst,aDoubles, nR );
        }
        else
        {
            aOclCalc.createBuffer32Bits( fpOclMatrixSrc, fpOclMatrixDst, nMatrixSize );
            for ( uint i = 0; i < nC; i++ )
                for ( uint j = 0; j < nR; j++ )
                    fpOclMatrixSrc[i*nC+j] = (float) aDoubles[j*nR+i];
            aOclCalc.oclHostMatrixInverse32Bits( "oclFormulaMtxInv", fpOclMatrixSrc, fpOclMatrixDst, aDoubles, nR );
        }
    }

    // TODO: Inverse this matrix and put the result back into xInv. Right now,
    // I'll just put the original, non-inversed matrix values back, just to
    // demonstrate how to put the values back after inversion.  There are two
    // ways to put the values back (depending on what the GPU output is).
    ScMatrixRef xInv(new ScMatrix(nR, nR, 0.0));

#if 0
    // One way is to put the whole value as one array. This method assumes
    // that the array size equals column x row, and is oriented column-wise.
    // This method is slightly more efficient than the second, but I wouldn't
    // expect too much of a difference.
    xInv->PutDouble(&aDoubles[0], aDoubles.size(), 0, 0);
#else
    // Another way is to put the values one column at a time.
    const double* p = &aDoubles[0];
    for( SCSIZE i = 0; i < nC; ++i )
    {
        xInv->PutDouble( p, nR, i, 0 );
        p += nR;
    }
#endif

    return xInv;
}
bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc, const ScAddress& rTopPos,
                                        const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode )
{
    generateRPNCode( rDoc, rTopPos, rCode );
    mnRowSize = xGroup->mnLength;
    fprintf( stderr,"mnRowSize at begin is ...%ld.\n",(long)mnRowSize );
    // The row quantity can be gotten from p2->GetArrayLength()
    int nOclOp = 0;
    const double * dpOclSrcData = NULL;
    const double * dpBinaryData = NULL;
    static OclCalc ocl_calc;
    unsigned int nSrcDataSize = 0;

    const double *dpResult = NULL;
    double *pResult = (double *)malloc(sizeof(double) * mnRowSize);
    double *dpSvDouble = NULL;
    bool isSample = false;

    mnSingleCount = 0;
    mnDoubleCount = 0;
    mnSvDoubleCount = 0;
    mnOperatorCount = 0;
    mnPositonLen = 0;
    if ( ocl_calc.getOpenclState() )
    {
        getPosition(rCode,xGroup,mnRowSize,mnpOclStartPos,mnpOclEndPos,&mnPositonLen);
        const formula::FormulaToken* p = rCode.FirstRPN();

        bool isSingle = false;
        int nCountNum=0;
        do
        {
            if ( ocPush != p->GetOpCode())
            {
                nOclOp = p->GetOpCode();
                mnOperatorGroup[mnOperatorCount++] = nOclOp;
            }
            else if( ocPush == p->GetOpCode() && formula::svSingleVectorRef == p->GetType() )
            {
                mnSingleCount++;
            }
            if ( ocPush == p->GetOpCode() && formula::svDouble == p->GetType() )
            {
                mnSvDoubleCount++;
            }
        } while ( NULL != ( p = rCode.NextRPN() ) );
        if( isGroundWater() )
        {
            isSample = true;
        }
        mnOperatorCount = 0;
        mnSingleCount = 0;
        mnSvDoubleCount = 0;
        p = rCode.FirstRPN();
        if(isSample)
        {
            do
            {
                if ( ocPush == p->GetOpCode() && formula::svDouble == p->GetType() )
                {
                    mdpSvdouble[mnSvDoubleCount++] = p->GetDouble();
                }
                else if( ocPush == p->GetOpCode() && formula::svDoubleVectorRef == p->GetType())
                {
                    const formula::DoubleVectorRefToken* pDvr = static_cast< const formula::DoubleVectorRefToken* >( p );
                    const std::vector<formula::VectorRefArray>& rArrays = pDvr->GetArrays();
                    uint rArraysSize = rArrays.size();
                    int nMoreColSize = 0;
                    DoubleVectorFormula *SvDoubleTemp = new DoubleVectorFormula();
                    if( rArraysSize > 1 )
                    {
                        double *dpMoreColData = NULL;
                        for ( uint loop=0; loop < rArraysSize; loop++ )
                        {
                            dpOclSrcData = rArrays[loop].mpNumericArray;
                            nSrcDataSize = pDvr->GetArrayLength();
                            nMoreColSize += nSrcDataSize;
                            dpMoreColData = (double *) realloc(dpMoreColData,nMoreColSize * sizeof(double));
                            for ( uint j = nMoreColSize - nSrcDataSize, i = 0; i < nSrcDataSize; i++, j++ )
                            {
                                dpMoreColData[j] = dpOclSrcData[i];
                            }
                        }
                        dpOclSrcData = dpMoreColData;
                        nSrcDataSize = nMoreColSize;
                    }
                    else
                    {
                        dpOclSrcData = rArrays[0].mpNumericArray;
                        nSrcDataSize = pDvr->GetArrayLength();
                        SvDoubleTemp->mdpInputData = dpOclSrcData;
                        SvDoubleTemp->mnInputDataSize = nSrcDataSize;
                        SvDoubleTemp->mnInputStartPosition = mnpOclStartPos[nCountNum*mnRowSize];
                        SvDoubleTemp->mnInputEndPosition = mnpOclEndPos[nCountNum*mnRowSize];
                        SvDoubleTemp->mnInputStartOffset = mnpOclStartPos[nCountNum*mnRowSize+1]-mnpOclStartPos[nCountNum*mnRowSize];
                        SvDoubleTemp->mnInputEndOffset = mnpOclEndPos[nCountNum*mnRowSize+1]-mnpOclEndPos[nCountNum*mnRowSize];
                        mDoubleArray[mnDoubleCount++] = SvDoubleTemp;
                        nCountNum++;
                    }
                }
                else if( ocPush == p->GetOpCode() && formula::svSingleVectorRef == p->GetType() )
                {
                    const formula::SingleVectorRefToken* pSvr = static_cast<const formula::SingleVectorRefToken*>( p );
                    dpBinaryData = pSvr->GetArray().mpNumericArray;
                    uint nArrayLen = pSvr->GetArrayLength();
                    SingleVectorFormula *SignleTemp = new SingleVectorFormula() ;
                    if(isSingle)
                    {
                        SignleTemp = mSingleArray[--mnSingleCount];
                        SignleTemp->mdpInputRightData = dpBinaryData;
                        SignleTemp->mnInputRightDataSize = nArrayLen;
                        SignleTemp->mnInputRightStartPosition = 0;
                        SignleTemp->mnInputRightOffset = 0;
                        isSingle = false;
                    }
                    else
                    {
                        SignleTemp = new SingleVectorFormula();
                        SignleTemp->mdpInputLeftData = dpBinaryData;
                        SignleTemp->mnInputLeftDataSize = nArrayLen;
                        SignleTemp->mdpInputRightData = NULL;
                        SignleTemp->mnInputRightDataSize = 0;
                        SignleTemp->mnInputLeftStartPosition = 0;
                        SignleTemp->mnInputLeftOffset = 0;
                        isSingle = true;
                    }
                    mSingleArray[mnSingleCount++] = SignleTemp;
                }
                else
                {
                    nOclOp = p->GetOpCode();
                    mnOperatorGroup[mnOperatorCount++] = nOclOp;
                }
            } while ( NULL != ( p = rCode.NextRPN() ) );
            if ( !chooseFunction( ocl_calc, pResult ) )
                return false;
            else
                dpResult = pResult;
        }
        else
        {
            agency aChooseAction;

            do
            {
                if ( ocPush == p->GetOpCode() && formula::svDouble == p->GetType() )
                {
                    dpSvDouble = (double *) malloc( sizeof(double ) * mnRowSize );
                    double dTempValue = p->GetDouble();
                    for ( uint i = 0; i < mnRowSize; i++ )
                        dpSvDouble[i] = dTempValue;
                    srdDataPush( new SourceData( dpSvDouble, mnRowSize ) );
                    collectDoublePointers( dpSvDouble );
                }
                else if( ocPush == p->GetOpCode() && formula::svDoubleVectorRef == p->GetType())
                {
                    const formula::DoubleVectorRefToken* pDvr = static_cast< const formula::DoubleVectorRefToken* >( p );
                    const std::vector<formula::VectorRefArray>& rArrays = pDvr->GetArrays();
                    unsigned int rArraysSize = rArrays.size();
                    int nMoreColSize = 0;
                    if(rArraysSize > 1)
                    {
                        double *dpMoreColData = NULL;
                        for( uint loop=0; loop < rArraysSize; loop++ )
                        {
                            dpOclSrcData = rArrays[loop].mpNumericArray;
                            nSrcDataSize = pDvr->GetArrayLength();
                            nMoreColSize += nSrcDataSize;
                            dpMoreColData = (double *) realloc(dpMoreColData,nMoreColSize * sizeof(double));
                            for(uint j=nMoreColSize-nSrcDataSize,i=0;i<nSrcDataSize;i++,j++)
                            {
                                dpMoreColData[j] = dpOclSrcData[i];
                            }
                        }
                        dpOclSrcData = dpMoreColData;
                        nSrcDataSize = nMoreColSize;
                        collectDoublePointers( dpMoreColData );
                    }
                    else
                    {
                        dpOclSrcData = rArrays[0].mpNumericArray;
                        nSrcDataSize = pDvr->GetArrayLength();
                    }
                    srdDataPush( new SourceData( dpOclSrcData,nSrcDataSize,rArraysSize ) );
                }
                else if( ocPush == p->GetOpCode() && formula::svSingleVectorRef == p->GetType() )
                {
                    const formula::SingleVectorRefToken* pSvr = static_cast<const formula::SingleVectorRefToken*>( p );
                    dpBinaryData = pSvr->GetArray().mpNumericArray;
                    nSrcDataSize = pSvr->GetArrayLength();
                    srdDataPush( new SourceData( dpBinaryData, nSrcDataSize ) );
                }
                else
                {
                    nOclOp = p->GetOpCode();
                    aChooseAction.calculate(nOclOp,mnRowSize,ocl_calc,mnpOclStartPos,mnpOclEndPos,this);
                    mnSingleCount = 0;
                    mnDoubleCount = 0;
                    mnSvDoubleCount = 0;
                    mnOperatorCount = 0;
                    mnPositonLen = 0;
                }
            } while ( NULL != ( p = rCode.NextRPN() ) );
            SourceData * sResult = srdDataPop();
            dpResult = sResult->getDouleData();
        }
        rDoc.SetFormulaResults( rTopPos, dpResult, mnRowSize );
        freeDoublePointers();
        if ( pResult )
        {
            free( pResult );
            pResult = NULL;
        }
        if ( mnpOclStartPos )
        {
            free( mnpOclStartPos );
            mnpOclStartPos = NULL;
        }
        if ( mnpOclEndPos )
        {
            free( mnpOclEndPos );
            mnpOclEndPos = NULL;
        }
        return true;
    } // getOpenclState() End
    else
        return false;
}

#if USE_GROUNDWATER_INTERPRETER

/// Special case of formula compiler for groundwatering
class FormulaGroupInterpreterGroundwater : public FormulaGroupInterpreterSoftware
{
    bool interpretCL(ScDocument& rDoc, const ScAddress& rTopPos,
                     const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode);

public:
    FormulaGroupInterpreterGroundwater() :
        FormulaGroupInterpreterSoftware()
    {
        fprintf(stderr,"\n\n ***** Groundwater Backend *****\n\n\n");
    }
    virtual ~FormulaGroupInterpreterGroundwater()
    {
    }

    virtual ScMatrixRef inverseMatrix(const ScMatrix& /* rMat */) { return ScMatrixRef(); }
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                           const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode);
};

#define RETURN_IF_FAIL(a,b) do { if (!(a)) { fprintf (stderr,b"\n"); return false; } } while (0)

#include "compiler.hxx"

bool FormulaGroupInterpreterGroundwater::interpretCL(ScDocument& rDoc, const ScAddress& rTopPos,
                                                     const ScFormulaCellGroupRef& xGroup,
                                                     ScTokenArray& rCode)
{
    generateRPNCode(rDoc, rTopPos, rCode);
    double delta = 0.0;
    // Inputs: both of length xGroup->mnLength
    OpCode eOp = ocNone; // type of operation: ocAverage, ocMax, ocMin
    const double *pArrayToSubtractOneElementFrom = NULL;
    const double *pGroundWaterDataArray = NULL;

    const formula::FormulaToken* p = rCode.FirstRPN();
    if ( p->GetType() == formula::svDouble && !getenv("SC_LCPU") )
    {
        delta = p->GetDouble();
        eOp = ocSub;
    }
    else
    {
        RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocPush && p->GetType() == formula::svDoubleVectorRef, "double vector ref expected");
        // Get the range reference vector.
        const formula::DoubleVectorRefToken* pDvr = static_cast<const formula::DoubleVectorRefToken*>(p);
        const std::vector<formula::VectorRefArray>& rArrays = pDvr->GetArrays();
        RETURN_IF_FAIL(rArrays.size() == 1, "unexpectedly large double ref array");
        RETURN_IF_FAIL(pDvr->GetArrayLength() == (size_t)xGroup->mnLength, "wrong double ref length");
        RETURN_IF_FAIL(pDvr->IsStartFixed() && pDvr->IsEndFixed(), "non-fixed ranges )");
        pGroundWaterDataArray = rArrays[0].mpNumericArray;

        // Function:
        p = rCode.NextRPN();
        RETURN_IF_FAIL(p != NULL, "no operator");
        eOp = p->GetOpCode();
        RETURN_IF_FAIL(eOp == ocAverage || eOp == ocMax || eOp == ocMin, "unexpected opcode - expected either average, max, or min");
    }

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocPush && p->GetType() == formula::svSingleVectorRef, "single vector ref expected");

    // Get the single reference vector.
    const formula::SingleVectorRefToken* pSvr = static_cast<const formula::SingleVectorRefToken*>(p);
    pArrayToSubtractOneElementFrom = pSvr->GetArray().mpNumericArray;
    RETURN_IF_FAIL(pSvr->GetArrayLength() == (size_t)xGroup->mnLength, "wrong single ref length");

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocSub, "missing subtract opcode");

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p == NULL, "there should be no more token");

    static OclCalc ocl_calc;

    // Here we have all the data we need to dispatch our openCL kernel [ I hope ]
    // so for:
    //   =AVERAGE(L$6:L$7701) - L6
    // we would get:
    //   eOp => ocAverage
    //   pGroundWaterDataArray => contains L$5:L$7701
    //   pArrayToSubtractOneElementFrom => contains L$5:L$7701 (ie. a copy)
    //   length of this array -> xGroup->mnLength

    fprintf (stderr, "Calculate !");

    double *pResult = ocl_calc.oclSimpleDeltaOperation( eOp, pGroundWaterDataArray,
                                                        pArrayToSubtractOneElementFrom,
                                                        (size_t) xGroup->mnLength, delta );
    RETURN_IF_FAIL(pResult != NULL, "buffer alloc / calculaton failed");

    // Insert the double data, in rResult[i] back into the document
    rDoc.SetFormulaResults(rTopPos, pResult, xGroup->mnLength);

    delete [] pResult;

    SAL_DEBUG ("exit cleanly !");
    return true;
}

bool FormulaGroupInterpreterGroundwater::interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                                                   const ScFormulaCellGroupRef& xGroup,
                                                   ScTokenArray& rCode)
{
    bool bComplete = interpretCL(rDoc, rTopPos, xGroup, rCode);
    if (!bComplete) // fallback to the (potentially) faster S/W formula group interpreter
        return FormulaGroupInterpreterSoftware::interpret(rDoc, rTopPos, xGroup, rCode);
    else
        return true;
}

#endif

} // namespace opencl

} // namespace sc

extern "C" {

SAL_DLLPUBLIC_EXPORT sc::FormulaGroupInterpreter* SAL_CALL createFormulaGroupOpenCLInterpreter()
{
#if USE_GROUNDWATER_INTERPRETER
    if (getenv("SC_GROUNDWATER"))
        return new sc::opencl::FormulaGroupInterpreterGroundwater();
#endif

    return new sc::opencl::FormulaGroupInterpreterOpenCL();
}

SAL_DLLPUBLIC_EXPORT size_t getOpenCLPlatformCount()
{
    return sc::opencl::getOpenCLPlatformCount();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL fillOpenCLInfo(sc::OpenclPlatformInfo* pInfos, size_t nInfoSize)
{
    const std::vector<sc::OpenclPlatformInfo>& rPlatforms = sc::opencl::fillOpenCLInfo();
    size_t n = std::min(rPlatforms.size(), nInfoSize);
    for (size_t i = 0; i < n; ++i)
        pInfos[i] = rPlatforms[i];
}

SAL_DLLPUBLIC_EXPORT bool SAL_CALL switchOpenClDevice(const OUString* pDeviceId, bool bAutoSelect)
{
    return sc::opencl::switchOpenclDevice(pDeviceId, bAutoSelect);
}

SAL_DLLPUBLIC_EXPORT void compileKernels(const OUString* pDeviceId)
{
    sc::opencl::compileKernels(pDeviceId);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
