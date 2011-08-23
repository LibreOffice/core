/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
namespace binfilter {

class SfxItemSet; 
struct SfxItemPropertyMap; 

class SwPaM;


namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
}}}
/* -----------------------------14.12.00 15:06--------------------------------

 ---------------------------------------------------------------------------*/
namespace SwUnoCursorHelper
{
    sal_Bool 					getCrsrPropertyValue(const SfxItemPropertyMap* pMap
                                        , SwPaM& rPam
                                        , ::com::sun::star::uno::Any *pAny
                                        , ::com::sun::star::beans::PropertyState& eState
                                        , const SwTxtNode* pNode = 0 );

    void 						GetCurPageStyle(SwPaM& rPaM, String &rString);

    inline sal_Bool 			IsStartOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetPoint()->nContent == 0;}
    inline sal_Bool 			IsEndOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetCntntNode() &&
                                            rUnoCrsr.GetPoint()->nContent == rUnoCrsr.GetCntntNode()->Len();}

    void 						resetCrsrPropertyValue(const SfxItemPropertyMap* pMap, SwPaM& rPam);
    void 						InsertFile(SwUnoCrsr* pUnoCrsr,
                                    const String& rFileName,
                                    const String& rFilterName,
                                    const String& rFilterOption,
                                    const String& rPassword);

    void 						getNumberingProperty(
                                    SwPaM& rPam,
                                    ::com::sun::star::beans::PropertyState& eState,
                                    ::com::sun::star::uno::Any *pAny );

    void 						setNumberingProperty(
                                    const ::com::sun::star::uno::Any& rValue,
                                    SwPaM& rPam);

    sal_Int16 					IsNodeNumStart(
                                    SwPaM& rPam,
                                    ::com::sun::star::beans::PropertyState& eState);

} //STRIP008 ;

} //namespace binfilter
#endif
