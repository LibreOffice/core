/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:02:25 $
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
    LanguageType        nLangError;     // nicht vorhandene Sprache
    sal_uInt16          nPageCount;     // Seitenanzahl fuer Progressanzeige
    sal_uInt16          nPageStart;     // 1. gepruefte Seite
    sal_Bool            bInSelection : 1; // Trennen von selektiertem Text
    sal_Bool            bShowError : 1; // nicht vorhandene Sprache melden
    sal_Bool            bAutomatic : 1; // Trennstellen ohne Rueckfrage einfuegen
    sal_Bool            bInfoBox : 1;   // display info-box when ending
    DECL_LINK( SpellError, LanguageType * );

protected:
    virtual void SpellStart( SvxSpellArea eSpell );
    virtual sal_Bool SpellContinue();
    virtual void SpellEnd( );
    virtual sal_Bool SpellMore();
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Hyphen einfuegen

public:
    SwHyphWrapper( SwView* pVw,
                   ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >  &rxHyph,
                   sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect );
    ~SwHyphWrapper();
};

#endif
