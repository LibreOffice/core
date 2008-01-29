/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocrsrhelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 09:22:26 $
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
#ifndef _UNOCRSRHELPER_HXX
#define _UNOCRSRHELPER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif


struct SfxItemPropertyMap;
class SwPaM;
class SwDoc;

namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
    namespace beans{
        struct PropertyValue;
    }
}}}
/* -----------------------------14.12.00 15:06--------------------------------

 ---------------------------------------------------------------------------*/
namespace SwUnoCursorHelper
{
    sal_Bool                    getCrsrPropertyValue(const SfxItemPropertyMap* pMap
                                        , SwPaM& rPam
                                        , com::sun::star::uno::Any *pAny
                                        , com::sun::star::beans::PropertyState& eState
                                        , const SwTxtNode* pNode = 0 );

    void                        GetCurPageStyle(SwPaM& rPaM, String &rString);

    inline sal_Bool             IsStartOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetPoint()->nContent == 0;}
    inline sal_Bool             IsEndOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetCntntNode() &&
                                            rUnoCrsr.GetPoint()->nContent == rUnoCrsr.GetCntntNode()->Len();}

    void                        resetCrsrPropertyValue(const SfxItemPropertyMap* pMap, SwPaM& rPam);
    void                        InsertFile(SwUnoCrsr* pUnoCrsr,
                                    const String& rURL,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rOptions
                                    ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException );

    void                        getNumberingProperty(
                                    SwPaM& rPam,
                                    com::sun::star::beans::PropertyState& eState,
                                    com::sun::star::uno::Any *pAny );

    void                        setNumberingProperty(
                                    const com::sun::star::uno::Any& rValue,
                                    SwPaM& rPam);

    sal_Int16                   IsNodeNumStart(
                                    SwPaM& rPam,
                                    com::sun::star::beans::PropertyState& eState);

    sal_Bool    DocInsertStringSplitCR(  SwDoc &rDoc,
                                    const SwPaM &rNewCursor, const String &rText );

} // namespace SwUnoCursorHelper

#endif
