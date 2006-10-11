/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentStylePoolAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-11 08:44:55 $
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

 #ifndef IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED
 #define IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED

 #ifndef _SAL_TYPES_H_
 #include <sal/types.h>
 #endif

 class SwTxtFmtColl;
 class SwCharFmt;
 class SwFmt;
 class SwFrmFmt;
 class String;
 class SwNumRule;
 class SwPageDesc;

 /** Access to the style pool
 */
 class IDocumentStylePoolAccess
 {
 public:

    /** Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        sie noch nicht, dann erzuege sie
        Ist der String-Pointer definiert, dann erfrage nur die
        Beschreibung der Attribute, !! es legt keine Vorlage an !!
    */
    virtual SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) = 0;

    /** return das geforderte automatische  Format - Basis-Klasse !
    */
    virtual SwFmt* GetFmtFromPool( sal_uInt16 nId ) = 0;

    /** returne das geforderte automatische Format
     */
    virtual SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId ) = 0;

    /**
     */
    virtual SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId ) = 0;

    /** returne die geforderte automatische Seiten-Vorlage
     */
    virtual SwPageDesc* GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) = 0;

    /**
     */
    virtual SwNumRule* GetNumRuleFromPool( sal_uInt16 nId ) = 0;

    /** pruefe, ob diese "Auto-Collection" in Dokument schon/noch benutzt wird
     */
    virtual bool IsPoolTxtCollUsed( sal_uInt16 nId ) const = 0;
    virtual bool IsPoolFmtUsed( sal_uInt16 nId ) const = 0;
    virtual bool IsPoolPageDescUsed( sal_uInt16 nId ) const = 0;

 protected:
    virtual ~IDocumentStylePoolAccess() {};
};

#endif // IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED

