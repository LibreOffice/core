/*************************************************************************
 *
 *  $RCSfile: hyp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 12:16:26 $
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

#ifndef _HYP_HXX
#define _HYP_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SVX_SPLWRAP_HXX //autogen
#include <svx/splwrap.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif

class SwView;

class SwHyphWrapper : public SvxSpellWrapper {
private:
    SwView* pView;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >  xHyph;
    Link            aOldLnk;
    sal_uInt32          nLangError;     // nicht vorhandene Sprache
    sal_uInt16          nPageCount;     // Seitenanzahl fuer Progressanzeige
    sal_uInt16          nPageStart;     // 1. gepruefte Seite
    sal_Bool            bInSelection : 1; // Trennen von selektiertem Text
    sal_Bool            bShowError : 1; // nicht vorhandene Sprache melden
    sal_Bool            bAutomatic : 1; // Trennstellen ohne Rueckfrage einfuegen
    DECL_LINK( SpellError, void * );

public:
    SwHyphWrapper( SwView* pVw,
                   ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >  &rxHyph,
                   sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect );

    ~SwHyphWrapper();
protected:
    virtual void SpellStart( SvxSpellArea eSpell );
    virtual sal_Bool SpellContinue();
    virtual void SpellEnd( );
    virtual sal_Bool SpellMore();
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Hyphen einfuegen
};

#endif
