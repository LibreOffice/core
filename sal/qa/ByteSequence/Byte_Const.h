#ifndef _BYTE_CONST_H_
#define _BYTE_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
const sal_Int32 kTestByteCount1 = 7;
const sal_Int32 kTestByteCount2 = 0;
const sal_Int32 kTestByteCount3 = 45;
const sal_Int32 kTestByteCount4 = 100;
const sal_Int32 kTestByteCount5 = 23;
const sal_Int32 kTestByteCount6 = 90;


sal_Int8 kTestByte = 100;
sal_Int8 kTestByte1 = 0;
sal_Int8 kTestByte2 = 1;
sal_Int8 kTestByte3 = 2;
sal_Int8 kTestByte4 = -98;

sal_Int8 kTestByte5[] = {kTestByte, kTestByte1, kTestByte2, kTestByte3, kTestByte4};

//------------------------------------------------------------------------

char kTestChar = 45;
char kTestChar0 = 0;
char kTestChar1 = 500;
char kTestChar2 = 78;
char kTestChar3 = -155;

sal_Int32 kTestSeqLen0 = 0;
sal_Int32 kTestSeqLen1 = 5;
sal_Int32 kTestSeqLen2 = 34;
sal_Int32 kTestSeqLen3 = 270;

sal_Sequence kTestEmptyByteSeq =
{
    1,              /* sal_Int32    refCount;   */
    kTestSeqLen0,       /* sal_Int32    length;     */
    { kTestChar0 }      /* sal_Unicode  buffer[1];  */
};

sal_Sequence kTestByteSeq1 =
{
    1,              /* sal_Int32    refCount;   */
    kTestSeqLen1,       /* sal_Int32    length;     */
    { kTestChar1 }      /* sal_Unicode  buffer[1];  */
};

sal_Sequence kTestByteSeq2 =
{
    3,              /* sal_Int32    refCount;   */
    kTestSeqLen2,       /* sal_Int32    length;     */
    { kTestChar2 }      /* sal_Unicode  buffer[1];  */
};

sal_Sequence kTestByteSeq3 =
{
    2,              /* sal_Int32    refCount;   */
    kTestSeqLen3,       /* sal_Int32    length;     */
    { kTestChar3 }      /* sal_Unicode  buffer[1];  */
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif /* _BYTE_CONST_H_ */


