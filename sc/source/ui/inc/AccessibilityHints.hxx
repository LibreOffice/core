/*************************************************************************
 *
 *  $RCSfile: AccessibilityHints.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2002-05-31 07:43:36 $
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

#ifndef SC_ACCESSIBILITYHINTS_HXX
#define SC_ACCESSIBILITYHINTS_HXX

#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif

#define SC_HINT_ACC_SIMPLE_START    SFX_HINT_USER00
#define SC_HINT_ACC_TABLECHANGED    SC_HINT_ACC_SIMPLE_START + 1
#define SC_HINT_ACC_CURSORCHANGED   SC_HINT_ACC_SIMPLE_START + 2
#define SC_HINT_ACC_VISAREACHANGED  SC_HINT_ACC_SIMPLE_START + 3
#define SC_HINT_ACC_EDITMODE        SC_HINT_ACC_SIMPLE_START + 4
#define SC_HINT_ACC_MAKEDRAWLAYER   SC_HINT_ACC_SIMPLE_START + 5

// ---------------------------------------------------------------------------

class ScAccGridViewChangeHint : public SfxHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xOldAccessible;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xNewAccessible;
    ScSplitPos  eOldGridWin;
    ScSplitPos  eNewGridWin;
public:
                TYPEINFO();
                ScAccGridViewChangeHint( ScSplitPos eOldGridWin, ScSplitPos eNewGridWin,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xOld,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xNew );
                ~ScAccGridViewChangeHint();

    ScSplitPos  GetOldGridWin() const { return eOldGridWin; }
    ScSplitPos  GetNewGridWin() const { return eNewGridWin; }
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetOldAccessible() const { return xOldAccessible; }
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetNewAccessible() const { return xOldAccessible; }
};

#endif
