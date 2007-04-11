/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:55:16 $
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

#ifndef _SBX_SBXOBJECT_HXX
#define _SBX_SBXOBJECT_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _SBXVAR_HXX
#include <basic/sbxvar.hxx>
#endif

///////////////////////////////////////////////////////////////////////////

class SbxProperty;
class SvDispatch;

class SbxObjectImpl;

class SbxObject : public SbxVariable, public SfxListener
{
    SbxObjectImpl* mpSbxObjectImpl; // Impl data

    SbxArray* FindVar( SbxVariable*, USHORT& );
    // AB 23.3.1997, Spezial-Methode fuer VCPtrRemove (s.u.)
    SbxArray* VCPtrFindVar( SbxVariable*, USHORT& );
protected:
    SbxArrayRef  pMethods;          // Methoden
    SbxArrayRef  pProps;            // Properties
    SbxArrayRef  pObjs;             // Objekte
    SbxProperty* pDfltProp;         // Default-Property
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
    void          SetClassName( const String &rNew ) { aClassName = rNew; }
    // Default-Property
    SbxProperty* GetDfltProperty();
    void SetDfltProperty( const String& r );
    void SetDfltProperty( SbxProperty* );
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
    // AB 23.3.1997, Spezial-Methode, gleichnamige Controls zulassen
    void VCPtrInsert( SbxVariable* );
    virtual void Remove( const String&, SbxClassType );
    virtual void Remove( SbxVariable* );
    // AB 23.3.1997, Loeschen per Pointer fuer Controls (doppelte Namen!)
    void VCPtrRemove( SbxVariable* );
    void SetPos( SbxVariable*, USHORT );

    // Makro-Recording
    virtual String GenerateSource( const String &rLinePrefix,
                                   const SbxObject *pRelativeTo );
    // Direktzugriff auf die Arrays
    SbxArray* GetMethods()      { return pMethods;  }
    SbxArray* GetProperties()   { return pProps;    }
    SbxArray* GetObjects()      { return pObjs;     }
    // Hooks
    virtual SvDispatch* GetSvDispatch();
    // Debugging
    void Dump( SvStream&, BOOL bDumpAll=FALSE );

    static void GarbageCollection( ULONG nObjects = 0 /* ::= all */ );
};

#ifndef __SBX_SBXOBJECTREF_HXX

#ifndef SBX_OBJECT_DECL_DEFINED
#define SBX_OBJECT_DECL_DEFINED
SV_DECL_REF(SbxObject)
#endif
SV_IMPL_REF(SbxObject)

#endif /* __SBX_SBXOBJECTREF_HXX */
#endif /* _SBX_SBXOBJECT_HXX */
