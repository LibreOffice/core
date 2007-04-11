/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbmod.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:52:25 $
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

#ifndef _SB_SBMOD_HXX
#define _SB_SBMOD_HXX

#include <basic/sbdef.hxx>
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _SBXDEF_HXX //autogen
#include <basic/sbxdef.hxx>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

class SbMethod;
class SbProperty;
class SbiRuntime;
class SbiBreakpoints;
class SbiImage;
class SbProcedureProperty;
class SbIfaceMapperMethod;

struct SbClassData;
class SbModuleImpl;

class SbModule : public SbxObject
{
    friend class    TestToolObj;    // somit können Module nach laden zur Laufzeit initialisiert werden
    friend class    SbiCodeGen;
    friend class    SbMethod;
    friend class    SbiRuntime;
    friend class    StarBASIC;
    friend class    SbClassModuleObject;

    SbModuleImpl*   mpSbModuleImpl;     // Impl data

protected:
    ::rtl::OUString aOUSource;
    String          aComment;
    SbiImage*       pImage;             // das Image
    SbiBreakpoints* pBreaks;            // Breakpoints
    SbClassData*    pClassData;

    void            StartDefinitions();
    SbMethod*       GetMethod( const String&, SbxDataType );
    SbProperty*     GetProperty( const String&, SbxDataType );
    SbProcedureProperty* GetProcedureProperty( const String&, SbxDataType );
    SbIfaceMapperMethod* GetIfaceMapperMethod( const String&, SbMethod* );
    void            EndDefinitions( BOOL=FALSE );
    USHORT          Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            GlobalRunInit( BOOL bBasicStart );  // fuer alle Module
    void            GlobalRunDeInit( void );
    const BYTE*     FindNextStmnt( const BYTE*, USHORT&, USHORT& ) const;
    const BYTE*     FindNextStmnt( const BYTE*, USHORT&, USHORT&,
                        BOOL bFollowJumps, const SbiImage* pImg=NULL ) const;
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual BOOL LoadCompleted();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    virtual ~SbModule();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMOD,2);
    TYPEINFO();
                    SbModule( const String& );
    virtual void    SetParent( SbxObject* );
    virtual void    Clear();

    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual const String&   GetSource() const;
    const ::rtl::OUString&  GetSource32() const;
    const String&   GetComment() const            { return aComment; }
    virtual void    SetSource( const String& r );
    void            SetSource32( const ::rtl::OUString& r );
    void            SetComment( const String& r );

    virtual BOOL    Compile();
    BOOL            Disassemble( String& rText );
    virtual BOOL    IsCompiled() const;
    const SbxObject* FindType( String aTypeName ) const;

    virtual BOOL    IsBreakable( USHORT nLine ) const;
    virtual USHORT  GetBPCount() const;
    virtual USHORT  GetBP( USHORT n ) const;
    virtual BOOL    IsBP( USHORT nLine ) const;
    virtual BOOL    SetBP( USHORT nLine );
    virtual BOOL    ClearBP( USHORT nLine );
    virtual void    ClearAllBP();

    // Zeilenbereiche von Subs
    virtual SbMethod*   GetFunctionForLine( USHORT );

    // Store only image, no source (needed for new password protection)
       BOOL StoreBinaryData( SvStream& );
       BOOL StoreBinaryData( SvStream&, USHORT nVer );
    BOOL LoadBinaryData( SvStream&, USHORT nVer );
    BOOL LoadBinaryData( SvStream& );
    BOOL ExceedsLegacyModuleSize();
    void fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = NULL ) const;
};

#ifndef __SB_SBMODULEREF_HXX
#define __SB_SBMODULEREF_HXX

SV_DECL_IMPL_REF(SbModule)

#endif

class SbClassModuleImpl;

// Object class for instances of class modules
class SbClassModuleObject : public SbModule
{
    SbClassModuleImpl* mpSbClassModuleImpl;

    SbModule*   mpClassModule;
    bool        mbInitializeEventDone;

public:
    TYPEINFO();
    SbClassModuleObject( SbModule* pClassModule );
    ~SbClassModuleObject();

    // Find ueberladen, um z.B. NameAccess zu unterstuetzen
    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );

    SbModule* getClassModule( void )
        { return mpClassModule; }

    void triggerInitializeEvent( void );
    void triggerTerminateEvent( void );
};

#ifndef __SB_SBCLASSMODULEREF_HXX
#define __SB_SBCLASSMODULEREF_HXX

SV_DECL_IMPL_REF(SbClassModuleObject);

#endif

#endif
