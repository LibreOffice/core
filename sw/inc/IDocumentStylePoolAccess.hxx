/*************************************************************************
 *
 *  $RCSfile: IDocumentStylePoolAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:14:04 $
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

