#ifndef INCLUDED_PICTTOBMPFLT_HXX
#define INCLUDED_PICTTOBMPFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

/* Transform PICT into the a Window BMP.

   Returns true if the conversion was successful false
   otherwise.
 */
bool PICTtoBMP(com::sun::star::uno::Sequence<sal_Int8>& aPict,
               com::sun::star::uno::Sequence<sal_Int8>& aBmp);

/* Transform a Windows BMP to a PICT.

   Returns true if the conversion was successful false
   otherwise.
 */
bool BMPtoPICT(com::sun::star::uno::Sequence<sal_Int8>& aBmp,
               com::sun::star::uno::Sequence<sal_Int8>& aPict);

#endif
