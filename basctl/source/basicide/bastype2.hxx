/*************************************************************************
 *
 *  $RCSfile: bastype2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:36 $
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
#ifndef _BASTYPE2_HXX
#define _BASTYPE2_HXX

#define _SVICNVW_HXX
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#define OBJTYPE_BASICMANAGER    1L
#define OBJTYPE_LIB             2L
#define OBJTYPE_MODULE          3L
#define OBJTYPE_METHOD          4L
#define OBJTYPE_METHODINOBJ     5L
#define OBJTYPE_OBJECT          6L
#define OBJTYPE_SUBOBJ          7L
#define OBJTYPE_PROPERTY        8L

#define BROWSEMODE_MODULES      0x01
#define BROWSEMODE_SUBS         0x02
#define BROWSEMODE_OBJS         0x04
#define BROWSEMODE_PROPS        0x08
#define BROWSEMODE_SUBOBJS      0x10

class BasicManager;
class SbMethod;
class SbxObject;
class SbModule;
class SvLBoxEntry;
class SbxVariable;

USHORT GetDialogSbxId();

class BasicTreeListBox : public SvTreeListBox
{
private:
    USHORT          nMode;
    ImageList       aImages;

    void            ScanSbxObject( SbxObject* pObj, SvLBoxEntry* pObjEntry );

protected:
    void                    ExpandTree( SvLBoxEntry* pRootEntry );
    virtual void            RequestingChilds( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    virtual SvLBoxEntry*    CloneEntry( SvLBoxEntry* pSource );
    SvLBoxEntry*            FindEntry( SvLBoxEntry* pParent, const String& rText, BYTE nType );
    virtual long            ExpandingHdl();

    void                    ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, StarBASIC* pLib );

public:
                    BasicTreeListBox( Window* pParent, const ResId& rRes );
                    ~BasicTreeListBox();

    void            ScanBasic( BasicManager* pBasMgr, const String& rName  );
    void            ScanAllBasics();
    void            UpdateEntries();

    void            ExpandAllTrees();

    BYTE            GetSelectedType();
    BasicManager*   GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrProp );
    BasicManager*   GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrPropOrSObj, String& rPropOrSubInSObj );
    BasicManager*   GetSbx( SvLBoxEntry* pEntry, String& rLib, String& rModOrObj, String& rSubOrPropOrSObj, String& rPropOrSubInSObj );
    BOOL            IsEntryProtected( SvLBoxEntry* pEntry );

    void            SetMode( USHORT nM ) { nMode = nM; }
    USHORT          GetMode() const { return nMode; }

    Image           GetImage( USHORT nId ) { return aImages.GetImage( nId ); }

    SbMethod*       FindMethod( SvLBoxEntry* pEntry );
    SbxObject*      FindObject( SvLBoxEntry* pEntry );
    SbModule*       FindModule( SvLBoxEntry* pEntry );
    SbxVariable*    FindVariable( SvLBoxEntry* pEntry );
    SvLBoxEntry*    FindLibEntry( StarBASIC* pLib );
};

/****************************************
    Zuordnung von Typen und Pointern in BasicEntrys:

    OBJTYPE_BASICMANAGER    BasicManagerEntry
    OBJTYPE_MODULE          BasicEntry
    OBJTYPE_OBJECT          BasicEntry
    OBJTYPE_METHOD          BasicEntry
    OBJTYPE_METHODINOBJ     BasicEntry
    OBJTYPE_SUBOBJ          BasicEntry
    OBJTYPE_PROPERTY        BasicEntry
    OBJTYPE_LIB             BasicEntry

******************************************/

class BasicEntry
{
private:
    BYTE            nType;

public:
                    BasicEntry( BYTE nT )               { nType = nT; }
                    BasicEntry( const BasicEntry& r )   { nType = r.nType; }
    virtual         ~BasicEntry();

    BYTE            GetType() const                     { return nType; }
};

class BasicManagerEntry : public BasicEntry
{
private:
    BasicManager*   pBasMgr;

public:
                    BasicManagerEntry( BasicManager* pMgr );
    virtual         ~BasicManagerEntry();

    BasicManager*   GetBasicManager() const { return pBasMgr; }
};

#endif  // _BASTYPE2_HXX
