/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IDocumentStylePoolAccess.hxx,v $
 * $Revision: 1.4 $
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

