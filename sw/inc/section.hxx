/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: section.hxx,v $
 * $Revision: 1.13.214.1 $
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

#ifndef _SECTION_HXX
#define _SECTION_HXX


#include <com/sun/star/uno/Sequence.h>
#include <sfx2/lnkbase.hxx>
#include <tools/rtti.hxx>
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#include <svtools/svarray.hxx>
#include <frmfmt.hxx>

// Forward Deklaration
class SwSectionFmt;
class SwDoc;
class SwSection;
class SwSectionNode;
class SwTOXBase;

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif

SV_DECL_PTRARR( SwSections, SwSection*, 0, 4 )

enum SectionType { CONTENT_SECTION,
                    TOX_HEADER_SECTION,
                    TOX_CONTENT_SECTION,
                    DDE_LINK_SECTION    = OBJECT_CLIENT_DDE,
                    FILE_LINK_SECTION   = OBJECT_CLIENT_FILE
/*
// verbleiben noch:
    OBJECT_CLIENT_SO            = 0x80,
    OBJECT_CLIENT_OLE           = 0x82,
    OBJECT_CLIENT_OLE_CACHE     = 0x83,
*/
                    };

enum LinkCreateType
{
    CREATE_NONE,            // nichts weiter tun
    CREATE_CONNECT,         // Link gleich connecten
    CREATE_UPDATE           // Link connecten und updaten
};


class SW_DLLPUBLIC SwSection : public SwClient
{
    // damit beim Anlegen/Loeschen von Frames das Flag richtig gepflegt wird!
    friend class SwSectionNode;
    // the "read CTOR" of SwSectionFrm have to change the Hiddenflag
    friend class SwSectionFrm;

    String sSectionNm;
    String sCondition;          // erstmal, vielleicht auch mal ein Feld ??
    String sLinkFileName,
           sLinkFilePassWd;     // JP 27.02.2001: must later changed to Sequence
    ::com::sun::star::uno::Sequence <sal_Int8> aPasswd;

    SwServerObjectRef refObj;   // falls DataServer -> Pointer gesetzt
    ::sfx2::SvBaseLinkRef refLink;

    SectionType eType;

    BOOL bProtectFlag : 1;      // Flags fuer schnelle Abfragen, wird ueber
                                // Attribut im Format gesetzt
    BOOL bHiddenFlag : 1;       // Flag: Absaetze versteckt ?
    // --> FME 2004-06-22 #114856# edit in readonly sections
    BOOL bEditInReadonlyFlag : 1;
    // <--
    BOOL bHidden : 1;           // alle Absaetze nicht sichtbar ?
    BOOL bCondHiddenFlag : 1;   // Hiddenflag fuer die Bedingung ?
    BOOL bConnectFlag : 1;      // Flag: "Verbindung zum Server" vorhanden?


    SW_DLLPRIVATE void _SetHiddenFlag( BOOL bHidden, BOOL bCondition );
    SW_DLLPRIVATE void _SetProtectFlag( BOOL bFlag ) { bProtectFlag = bFlag; }
    /* SW_DLLPUBLIC */ BOOL _IsProtect() const;

    // --> FME 2004-06-22 #114856# edit in readonly sections
    void _SetEditInReadonlyFlag( BOOL bFlag ) { bEditInReadonlyFlag = bFlag; }
    BOOL _IsEditInReadonly() const;
    // <--

public:
    TYPEINFO();     // rtti

    SwSection( SectionType eType, const String& rName,
                SwSectionFmt* pFmt = 0 );
    ~SwSection();

    // kopiere nur die Daten der Section!
    // Ableitung bleibt (beim Left) erhalten.
    SwSection& operator=( const SwSection& );
    BOOL operator==( const SwSection& rCmp ) const;

    const String& GetName() const           { return sSectionNm; }
    void SetName( const String& rName )     { sSectionNm = rName; }
    SectionType GetType() const     { return eType; }
    void SetType( SectionType eNew )        { eType = eNew; }

    SwSectionFmt* GetFmt()          { return (SwSectionFmt*)pRegisteredIn; }
    SwSectionFmt* GetFmt() const    { return (SwSectionFmt*)pRegisteredIn; }

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    // setze die Hidden/Protected -> gesamten Baum updaten !
    // (Attribute/Flags werden gesetzt/erfragt)
    BOOL IsHidden() const { return bHidden; }
    BOOL IsProtect() const { return GetFmt() ? _IsProtect()
                                            : IsProtectFlag(); }

    // --> FME 2004-06-22 #114856# edit in readonly sections
    BOOL IsEditInReadonly()const { return GetFmt() ? _IsEditInReadonly() : IsEditInReadonlyFlag(); }
    void SetEditInReadonly( BOOL bFlag = TRUE );
    // <--

    void SetHidden( BOOL bFlag = TRUE );
    void SetProtect( BOOL bFlag = TRUE );

    // erfrage die internen Flags (Zustand inklusive Parents nicht, was
    // aktuell an der Section gesetzt ist!!)
    BOOL IsHiddenFlag() const { return bHiddenFlag; }
    BOOL IsProtectFlag() const { return bProtectFlag; }
    // --> FME 2004-06-22 #114856# edit in readonly sections
    BOOL IsEditInReadonlyFlag() const { return bEditInReadonlyFlag; }
    // <--

    void SetCondHidden( BOOL bFlag = TRUE );
    BOOL IsCondHidden() const { return bCondHiddenFlag; }
    // erfrage (auch ueber die Parents), ob diese Section versteckt sein soll.
    BOOL CalcHiddenFlag() const;


    inline SwSection* GetParent() const;

    // setze/erfrage die Bedingung
    const String& GetCondition() const          { return sCondition; }
    void SetCondition( const String& rNew )     { sCondition = rNew; }

    // setze/erfrage den gelinkten FileNamen
    const String& GetLinkFileName() const;
    void SetLinkFileName( const String& rNew, const String* pPassWd = 0 );
    // Passwort des gelinkten Files (nur waehrend der Laufzeit gueltig!)
    const String& GetLinkFilePassWd() const         { return sLinkFilePassWd; }
    void SetLinkFilePassWd( const String& rS )      { sLinkFilePassWd = rS; }

    // get / set password of this section
    const ::com::sun::star::uno::Sequence <sal_Int8>&
            GetPasswd() const               { return aPasswd; }
    void SetPasswd( const ::com::sun::star::uno::Sequence <sal_Int8>& rNew )
                                            { aPasswd = rNew; }

    // Daten Server-Methoden
    void SetRefObject( SwServerObject* pObj );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }
    BOOL IsServer() const                       {  return refObj.Is(); }

    // Methoden fuer gelinkte Bereiche
    USHORT GetUpdateType() const        { return refLink->GetUpdateMode();  }
    void SetUpdateType( USHORT nType )  { refLink->SetUpdateMode( nType );  }

    BOOL IsConnected() const        { return refLink.Is(); }
    void UpdateNow()                { refLink->Update(); }
    void Disconnect()               { refLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *refLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *refLink; }

    void CreateLink( LinkCreateType eType );

    void MakeChildLinksVisible( const SwSectionNode& rSectNd );

    BOOL IsLinkType() const
        { return DDE_LINK_SECTION == eType || FILE_LINK_SECTION == eType; }

    // Flags fuer UI - Verbindung geklappt?
    BOOL IsConnectFlag() const                  { return bConnectFlag; }
    void SetConnectFlag( BOOL bFlag = TRUE )    { bConnectFlag = bFlag; }

    // return the TOX base class if the section is a TOX section
    const SwTOXBase* GetTOXBase() const;

    // --> OD 2007-02-14 #b6521322#
    void BreakLink();
    // <--
private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwSection( const SwSection& );
    // @@@ but copy assignment "SwSection & operator= ( const SwSection& )" is public? @@@
};


enum SectionSort { SORTSECT_NOT, SORTSECT_NAME, SORTSECT_POS };

class SW_DLLPUBLIC SwSectionFmt : public SwFrmFmt
{
    friend class SwDoc;

    /* SW_DLLPUBLIC */ SwSection* _GetSection() const;
    SW_DLLPRIVATE void UpdateParent();      // Parent wurde veraendert

protected:
    SwSectionFmt( SwSectionFmt* pDrvdFrm, SwDoc *pDoc );

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
    ~SwSectionFmt();

    //Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
    virtual void DelFrms();

    //Erzeugt die Ansichten
    virtual void MakeFrms();

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
        // erfrage vom Format Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    SwSection* GetSection() const { return (SwSection*)_GetSection(); }
    inline SwSectionFmt* GetParent() const;
    inline SwSection* GetParentSection() const;

    // alle Sections, die von dieser abgeleitet sind
    //  - sortiert nach : Name oder Position oder unsortiert
    //  - alle oder nur die, die sich im normalten Nodes-Array befinden
    USHORT GetChildSections( SwSections& rArr,
                            SectionSort eSort = SORTSECT_NOT,
                            BOOL bAllSections = TRUE ) const;

    // erfrage, ob sich die Section im Nodes-Array oder UndoNodes-Array
    // befindet.
    BOOL IsInNodesArr() const;

          SwSectionNode* GetSectionNode( BOOL bAlways = FALSE );
    const SwSectionNode* GetSectionNode( BOOL bAlways = FALSE ) const
    {   return ((SwSectionFmt*)this)->GetSectionNode( bAlways ); }

    // ist die Section eine gueltige fuers GlobalDocument?
    const SwSection* GetGlobalDocSection() const;
};

// -------------- inlines ---------------------------------

inline SwSection* SwSection::GetParent() const
{
    SwSectionFmt* pFmt = GetFmt();
    SwSection* pRet = 0;
    if( pFmt )
        pRet = pFmt->GetParentSection();
    return pRet;
}

inline SwSectionFmt* SwSectionFmt::GetParent() const
{
    SwSectionFmt* pRet = 0;
    if( GetRegisteredIn() )
        pRet = PTR_CAST( SwSectionFmt, GetRegisteredIn() );
    return pRet;
}

inline SwSection* SwSectionFmt::GetParentSection() const
{
    SwSectionFmt* pParent = GetParent();
    SwSection* pRet = 0;
    if( pParent )
        pRet = pParent->_GetSection();
    return pRet;
}


#endif /* _SECTION_HXX */
