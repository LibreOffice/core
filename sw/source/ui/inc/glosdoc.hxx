/*************************************************************************
 *
 *  $RCSfile: glosdoc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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
#ifndef _GLOSDOC_HXX
#define _GLOSDOC_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTGROUP_HPP_
#include <com/sun/star/text/XAutoTextGroup.hpp>
#endif

class SwTextBlocks;
class SvStrings;

typedef com::sun::star::uno::Reference<com::sun::star::text::XAutoTextGroup>* XAutoTextGroupPtr;
SV_DECL_PTRARR_DEL(XAutoTextGroupPtrArr, XAutoTextGroupPtr, 4, 4)

typedef com::sun::star::uno::Reference<com::sun::star::uno::XInterface>* XInterfacePtr;
SV_DECL_PTRARR_DEL(XAutoTextEntryPtrArr, XInterfacePtr, 4, 4)

#define GLOS_DELIM (sal_Unicode)'*'

// CLASS -----------------------------------------------------------------
class SwGlossaries
{
    XAutoTextGroupPtrArr    aGlosGroupArr;
    XAutoTextEntryPtrArr    aGlosEntryArr;
    String                  aPath;
    String                  sOldErrPath;
    String                  sErrPath;
    SvStrings               *pPathArr;
    SvStrings               *pGlosArr;
    BOOL                    bError;

    SwTextBlocks*   GetGlosDoc(const String &rName, BOOL bCreate = TRUE) const;
    SvStrings       *GetNameList();
public:
    SwGlossaries();
    ~SwGlossaries();

    XAutoTextGroupPtrArr&   GetUnoGroupArray()  { return aGlosGroupArr; }
    XAutoTextEntryPtrArr&   GetUnoEntryArray()  { return aGlosEntryArr; }

    USHORT                  GetGroupCnt();
    String                  GetGroupName(USHORT );
    String                  GetGroupTitle( const String& rGroupName );

    BOOL            FindGroupName(String & rGroup);

    SwTextBlocks*   GetGroupDoc(const String &rName,
                                BOOL bCreate = FALSE) const;
    SwTextBlocks*   GetDefGroupDoc() const {return GetGroupDoc(GetDefName());}
    void            PutGroupDoc(SwTextBlocks *pBlock);
    static String   GetDefName();
    static String   GetExtension();

    BOOL            NewGroupDoc(String &rGroupName, const String& rTitle);
    BOOL            RenameGroupDoc(const String& sOldGroup, String& sNewGroup, const String& rNewTitle);
    BOOL            DelGroupDoc(const String &);
    void            EditGroupDoc(const String &rGrpName, const String& rShortName );
    void            SaveGroupDoc(const String &rGrpName, const String& rLongName );
    void            UpdateGlosPath(BOOL bFull);
    void            ShowError();
    inline ULONG    IsGlosPathErr() { return bError; }
    const SvStrings*    GetPathArray() const {return pPathArr;}
};

#endif // _GLOSDOC_HXX
