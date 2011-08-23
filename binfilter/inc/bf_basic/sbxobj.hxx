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

#ifndef _SBX_SBXOBJECT_HXX
#define _SBX_SBXOBJECT_HXX

#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif

#ifndef _SBXVAR_HXX
#include "sbxvar.hxx"
#endif

///////////////////////////////////////////////////////////////////////////

class SvDispatch;

namespace binfilter {

class SbxProperty;
class SbxObjectImpl;

class SbxObject : public SbxVariable, public SfxListener
{
    SbxObjectImpl* mpSbxObjectImpl;	// Impl data

    SbxArray* FindVar( SbxVariable*, USHORT& );
protected:
    SbxArrayRef  pMethods;			// Methoden
    SbxArrayRef  pProps;			// Properties
    SbxArrayRef  pObjs;				// Objekte
    SbxProperty* pDfltProp;			// Default-Property
    String       aClassName;        // Klassenname
    String       aDfltPropName;
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual ~SbxObject();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_OBJECT,1);
    TYPEINFO();
    SbxObject( const String& rClassname );
    SbxObject( const SbxObject& );
    SbxObject& operator=( const SbxObject& );
    virtual SbxDataType GetType() const;
    virtual SbxClassType GetClass() const;
    virtual void Clear();

    virtual BOOL  IsClass( const String& ) const;
    const String& GetClassName() const { return aClassName; }
    void		  SetClassName( const String &rNew ) { aClassName = rNew; }
    // Default-Property
    SbxProperty* GetDfltProperty();
    // Suchen eines Elements
    virtual SbxVariable* FindUserData( UINT32 nUserData );
    virtual SbxVariable* Find( const String&, SbxClassType );
    SbxVariable* FindQualified( const String&, SbxClassType );
    // Quick-Call-Interface fuer Methoden
    virtual BOOL Call( const String&, SbxArray* = NULL );
    // Execution von DDE-Kommandos
    SbxVariable* Execute( const String& );
    // Elemente verwalten
    virtual BOOL GetAll( SbxClassType ) { return TRUE; }
    SbxVariable* Make( const String&, SbxClassType, SbxDataType );
    virtual SbxObject* MakeObject( const String&, const String& );
    virtual void Insert( SbxVariable* );
    // AB 23.4.1997, Optimierung, Einfuegen ohne Ueberpruefung auf doppelte
    // Eintraege und ohne Broadcasts, wird nur in SO2/auto.cxx genutzt
    void QuickInsert( SbxVariable* );
    virtual void Remove( const String&, SbxClassType );
    virtual void Remove( SbxVariable* );

    // Makro-Recording
    virtual String GenerateSource( const String &rLinePrefix,
                                   const SbxObject *pRelativeTo );
    // Direktzugriff auf die Arrays
    SbxArray* GetMethods()		{ return pMethods;	}
    SbxArray* GetProperties()	{ return pProps; 	}
    SbxArray* GetObjects()		{ return pObjs; 	}
    // Hooks
    virtual SvDispatch* GetSvDispatch();
    // Debugging
    void Dump( SvStream&, BOOL bDumpAll=FALSE );
};

#ifndef __SBX_SBXOBJECTREF_HXX

#ifndef SBX_OBJECT_DECL_DEFINED
#define SBX_OBJECT_DECL_DEFINED
SV_DECL_REF(SbxObject)
#endif
SV_IMPL_REF(SbxObject)

#endif /* __SBX_SBXOBJECTREF_HXX */

}

#endif /* _SBX_SBXOBJECT_HXX */
