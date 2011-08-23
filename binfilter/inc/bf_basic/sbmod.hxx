/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SB_SBMOD_HXX
#define _SB_SBMOD_HXX

#include "sbdef.hxx"
#include "sbxobj.hxx"
#include "sbxdef.hxx"
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

namespace binfilter {
class SbMethod;
class SbProperty;
class SbiRuntime;
class SbiImage;
class SbProcedureProperty;
class SbIfaceMapperMethod;

struct SbClassData;
class SbModuleImpl;

class SbModule : public SbxObject
{
    friend class	TestToolObj;	// somit können Module nach laden zur Laufzeit initialisiert werden
    friend class	SbiCodeGen;
    friend class	SbMethod;
    friend class	SbiRuntime;
    friend class	StarBASIC;
    friend class	SbClassModuleObject;

    SbModuleImpl*	mpSbModuleImpl;		// Impl data

protected:
    ::rtl::OUString	aOUSource;
    String			aComment;
    SbiImage*		pImage;				// das Image
    SbClassData*	pClassData;

    void			StartDefinitions();
    SbMethod*		GetMethod( const String&, SbxDataType );
    void			EndDefinitions( BOOL=FALSE );
    USHORT 			Run( SbMethod* );
    const BYTE* 	FindNextStmnt( const BYTE*, USHORT&, USHORT& ) const;
    const BYTE* 	FindNextStmnt( const BYTE*, USHORT&, USHORT&, 
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
    virtual void	SetParent( SbxObject* );
    virtual void 	Clear();

    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual const String& 	GetSource() const;
    const ::rtl::OUString& 	GetSource32() const;
    const String&	GetComment() const            { return aComment; }
    virtual void	SetSource( const String& r );
    void	        SetSource32( const ::rtl::OUString& r );

    virtual BOOL	Compile();
    BOOL 			Disassemble( String& rText );
    virtual BOOL	IsCompiled() const;
    const SbxObject* FindType( String aTypeName ) const;

    virtual BOOL	IsBreakable( USHORT nLine ) const;
    virtual USHORT	GetBPCount() const;
    virtual USHORT	GetBP( USHORT n ) const;
    virtual BOOL	IsBP( USHORT nLine ) const;
    virtual BOOL	SetBP( USHORT nLine );
    virtual BOOL	ClearBP( USHORT nLine );
    virtual void	ClearAllBP();

    // Zeilenbereiche von Subs
    virtual SbMethod*	GetFunctionForLine( USHORT );

    BOOL LoadBinaryData( SvStream&, USHORT nVer );
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

    SbModule*	mpClassModule;
    bool		mbInitializeEventDone;

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

}

#endif
