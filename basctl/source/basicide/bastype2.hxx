/*************************************************************************
 *
 *  $RCSfile: bastype2.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:03:57 $
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

#include <memory>

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif

#define _SVICNVW_HXX
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SBXITEM_HXX
#include <sbxitem.hxx>
#endif
#ifndef _BASOBJ_HXX
#include "basobj.hxx"
#endif

enum BasicEntryType { OBJ_TYPE_UNKNOWN, OBJ_TYPE_SHELL, OBJ_TYPE_LIBRARY, OBJ_TYPE_MODULE, OBJ_TYPE_DIALOG, OBJ_TYPE_METHOD };

#define BROWSEMODE_MODULES      0x01
#define BROWSEMODE_SUBS         0x02
#define BROWSEMODE_DIALOGS      0x04

class SbMethod;
class SbxObject;
class SbModule;
class SvLBoxEntry;
class SbxVariable;
class String;


class BasicEntry
{
private:
    BasicEntryType  m_eType;

public:
                    BasicEntry( BasicEntryType eType )  { m_eType = eType; }
                    BasicEntry( const BasicEntry& r )   { m_eType = r.m_eType; }
    virtual         ~BasicEntry();

    BasicEntryType  GetType() const                     { return m_eType; }
};

class BasicShellEntry : public BasicEntry
{
private:
    SfxObjectShell* m_pShell;
    LibraryLocation m_eLocation;

public:
                    BasicShellEntry( SfxObjectShell* pShell, LibraryLocation eLocation, BasicEntryType eType = OBJ_TYPE_SHELL );
    virtual         ~BasicShellEntry();

    SfxObjectShell* GetShell() const { return m_pShell; }
    LibraryLocation GetLocation() const { return m_eLocation; }
};

class BasicLibEntry : public BasicShellEntry
{
private:
    String          m_aLibName;

public:
                    BasicLibEntry( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, BasicEntryType eType = OBJ_TYPE_LIBRARY );
    virtual         ~BasicLibEntry();

    const String&   GetLibName() const { return m_aLibName; }
};

class BasicEntryDescriptor
{
    SfxObjectShell*         m_pShell;
    LibraryLocation         m_eLocation;
    String                  m_aLibName;
    String                  m_aName;
    String                  m_aMethodName;
    BasicEntryType          m_eType;

public:
                            BasicEntryDescriptor();
                            BasicEntryDescriptor( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, const String& rName, BasicEntryType eType );
                            BasicEntryDescriptor( SfxObjectShell* pShell, LibraryLocation eLocation, const String& rLibName, const String& rName, const String& rMethodName, BasicEntryType eType );
    virtual                 ~BasicEntryDescriptor();

                            BasicEntryDescriptor( const BasicEntryDescriptor& rDesc );
    BasicEntryDescriptor&   operator=( const BasicEntryDescriptor& rDesc );
    bool                    operator==( const BasicEntryDescriptor& rDesc ) const;

    SfxObjectShell*         GetShell() const { return m_pShell; }
    void                    SetShell( SfxObjectShell* pShell ) { m_pShell = pShell; }

    LibraryLocation         GetLocation() const { return m_eLocation; }
    void                    SetLocation( LibraryLocation eLocation ) { m_eLocation = eLocation; }

    const String&           GetLibName() const { return m_aLibName; }
    void                    SetLibName( const String& aLibName ) { m_aLibName = aLibName; }

    const String&           GetName() const { return m_aName; }
    void                    SetName( const String& aName ) { m_aName = aName; }

    const String&           GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const String& aMethodName ) { m_aMethodName = aMethodName; }

    BasicEntryType          GetType() const { return m_eType; }
    void                    SetType( BasicEntryType eType ) { m_eType = eType; }
};


/****************************************
    Zuordnung von Typen und Pointern in BasicEntrys:

    OBJ_TYPE_SHELL           BasicShellEntry
    OBJ_TYPE_LIBRARY         BasicEntry
    OBJ_TYPE_MODULE          BasicEntry
    OBJ_TYPE_DIALOG          BasicEntry
    OBJ_TYPE_METHOD          BasicEntry

******************************************/

class BasicTreeListBox : public SvTreeListBox
{
private:
    USHORT          nMode;

    void            SetEntryBitmaps( SvLBoxEntry * pEntry, const Image& rImage, const Image& rImageHC );

protected:
    void                    ExpandTree( SvLBoxEntry* pRootEntry );
    virtual void            RequestingChilds( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    virtual SvLBoxEntry*    CloneEntry( SvLBoxEntry* pSource );
    virtual long            ExpandingHdl();

    void                    ImpCreateLibEntries( SvLBoxEntry* pShellRootEntry, SfxObjectShell* pShell, LibraryLocation eLocation );
    void                    ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, SfxObjectShell* pShell, const String& rLibName );

public:
                    BasicTreeListBox( Window* pParent, const ResId& rRes );
                    ~BasicTreeListBox();

    void            ScanEntry( SfxObjectShell* pShell, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();

    void            ExpandAllTrees();

    BOOL            IsEntryProtected( SvLBoxEntry* pEntry );

    void            SetMode( USHORT nM ) { nMode = nM; }
    USHORT          GetMode() const { return nMode; }

    SbModule*       FindModule( SvLBoxEntry* pEntry );
    SbxVariable*    FindVariable( SvLBoxEntry* pEntry );
    SvLBoxEntry*    FindRootEntry( SfxObjectShell* pShell, LibraryLocation eLocation );
    SvLBoxEntry*    FindEntry( SvLBoxEntry* pParent, const String& rText, BasicEntryType eType );

    BasicEntryDescriptor    GetEntryDescriptor( SvLBoxEntry* pEntry );

    USHORT          ConvertType( BasicEntryType eType );
    bool            IsValidEntry( SvLBoxEntry* pEntry );

    SvLBoxEntry*    AddEntry( const String& rText, const Image& rImage, const Image& rImageHC,
                              SvLBoxEntry* pParent, bool bChildrenOnDemand,
                              std::auto_ptr< BasicEntry > aUserData );

    String          GetRootEntryName( SfxObjectShell* pShell, LibraryLocation eLocation );
    void            GetRootEntryBitmaps( SfxObjectShell* pShell, Image& rImage, Image& rImageHC );

    void            SetCurrentEntry( BasicEntryDescriptor& rDesc );
};

#endif  // _BASTYPE2_HXX
