/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Inflater.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:56:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _INFLATER_HXX_
#define _INFLATER_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

extern "C"
{
    typedef struct z_stream_s z_stream;
}
class Inflater
{
protected:
    sal_Bool                bFinish, bFinished, bSetParams, bNeedDict;
    sal_Int32               nOffset, nLength;
    z_stream*               pStream;
    com::sun::star::uno::Sequence < sal_Int8 >  sInBuffer;
    sal_Int32   doInflateBytes (com::sun::star::uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    Inflater(sal_Bool bNoWrap = sal_False);
    ~Inflater();
    void SAL_CALL setInputSegment( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void SAL_CALL setInput( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer );
    void SAL_CALL setDictionarySegment( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void SAL_CALL setDictionary( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer );
    sal_Int32 SAL_CALL getRemaining(  );
    sal_Bool SAL_CALL needsInput(  );
    sal_Bool SAL_CALL needsDictionary(  );
    void SAL_CALL finish(  );
    sal_Bool SAL_CALL finished(  );
    sal_Int32 SAL_CALL doInflateSegment( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    sal_Int32 SAL_CALL doInflate( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer );
    sal_Int32 SAL_CALL getAdler(  );
    sal_Int32 SAL_CALL getTotalIn(  );
    sal_Int32 SAL_CALL getTotalOut(  );
    void SAL_CALL reset(  );
    void SAL_CALL end(  );
};

#endif
