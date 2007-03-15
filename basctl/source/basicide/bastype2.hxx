/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bastype2.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 15:55:20 $
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
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
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

enum BasicEntryType { OBJ_TYPE_UNKNOWN, OBJ_TYPE_DOCUMENT, OBJ_TYPE_LIBRARY, OBJ_TYPE_MODULE, OBJ_TYPE_DIALOG, OBJ_TYPE_METHOD };

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

class BasicDocumentEntry : public BasicEntry
{
private:
    ScriptDocument      m_aDocument;
    LibraryLocation     m_eLocation;

public:
                    BasicDocumentEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, BasicEntryType eType = OBJ_TYPE_DOCUMENT );
    virtual         ~BasicDocumentEntry();

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    LibraryLocation GetLocation() const { return m_eLocation; }
};

class BasicLibEntry : public BasicDocumentEntry
{
private:
    String          m_aLibName;

public:
                    BasicLibEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, const String& rLibName, BasicEntryType eType = OBJ_TYPE_LIBRARY );
    virtual         ~BasicLibEntry();

    const String&   GetLibName() const { return m_aLibName; }
};

class BasicEntryDescriptor
{
    ScriptDocument          m_aDocument;
    LibraryLocation         m_eLocation;
    String                  m_aLibName;
    String                  m_aName;
    String                  m_aMethodName;
    BasicEntryType          m_eType;

public:
                            BasicEntryDescriptor();
                            BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const String& rLibName, const String& rName, BasicEntryType eType );
                            BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const String& rLibName, const String& rName, const String& rMethodName, BasicEntryType eType );
    virtual                 ~BasicEntryDescriptor();

                            BasicEntryDescriptor( const BasicEntryDescriptor& rDesc );
    BasicEntryDescriptor&   operator=( const BasicEntryDescriptor& rDesc );
    bool                    operator==( const BasicEntryDescriptor& rDesc ) const;

    const ScriptDocument&
                            GetDocument() const { return m_aDocument; }
    void                    SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

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

    OBJ_TYPE_DOCUMENT        BasicDocumentEntry
    OBJ_TYPE_LIBRARY         BasicEntry
    OBJ_TYPE_MODULE          BasicEntry
    OBJ_TYPE_DIALOG          BasicEntry
    OBJ_TYPE_METHOD          BasicEntry

******************************************/

class BasicTreeListBox : public SvTreeListBox, public SfxListener
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

    void                    ImpCreateLibEntries( SvLBoxEntry* pShellRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                    ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, const ScriptDocument& rDocument, const String& rLibName );

    using                   Control::Notify;
    virtual void            SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType );

public:
                    BasicTreeListBox( Window* pParent, const ResId& rRes );
                    ~BasicTreeListBox();

    void            ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();

    void            ExpandAllTrees();

    BOOL            IsEntryProtected( SvLBoxEntry* pEntry );

    void            SetMode( USHORT nM ) { nMode = nM; }
    USHORT          GetMode() const { return nMode; }

    SbModule*       FindModule( SvLBoxEntry* pEntry );
    SbxVariable*    FindVariable( SvLBoxEntry* pEntry );
    SvLBoxEntry*    FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    SvLBoxEntry*    FindEntry( SvLBoxEntry* pParent, const String& rText, BasicEntryType eType );

    BasicEntryDescriptor    GetEntryDescriptor( SvLBoxEntry* pEntry );

    USHORT          ConvertType( BasicEntryType eType );
    bool            IsValidEntry( SvLBoxEntry* pEntry );

    SvLBoxEntry*    AddEntry( const String& rText, const Image& rImage, const Image& rImageHC,
                              SvLBoxEntry* pParent, bool bChildrenOnDemand,
                              std::auto_ptr< BasicEntry > aUserData );

    String          GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage, Image& rImageHC );

    void            SetCurrentEntry( BasicEntryDescriptor& rDesc );
};

#endif  // _BASTYPE2_HXX
