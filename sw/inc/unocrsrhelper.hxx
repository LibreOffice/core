/*************************************************************************
 *
 *  $RCSfile: unocrsrhelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 15:39:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
class SfxItemSet;
class SwDoc;

namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
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
                                    const String& rFileName,
                                    const String& rFilterName,
                                    const String& rFilterOption,
                                    const String& rPassword);

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

};

#endif
