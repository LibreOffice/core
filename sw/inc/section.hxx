/*************************************************************************
 *
 *  $RCSfile: section.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-02 14:35:07 $
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

#ifndef _SECTION_HXX
#define _SECTION_HXX


#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _LNKBASE_HXX
#include <so3/lnkbase.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _SO2REF_HXX
#include <so3/so2ref.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif

// Forward Deklaration
class SwSectionFmt;
class SwDoc;
class SwSection;
class SwTOXBase;

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
class SvPseudoObject;
SO2_DECL_REF( SwServerObject )
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


class SwSection : public SwClient
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
    SvBaseLinkRef refLink;

    SectionType eType;

    BOOL bProtectFlag : 1;      // Flags fuer schnelle Abfragen, wird ueber
                                // Attribut im Format gesetzt
    BOOL bHiddenFlag : 1;       // Flag: Absaetze versteckt ?
    BOOL bHidden : 1;           // alle Absaetze nicht sichtbar ?
    BOOL bCondHiddenFlag : 1;   // Hiddenflag fuer die Bedingung ?
    BOOL bConnectFlag : 1;      // Flag: "Verbindung zum Server" vorhanden?


    void _SetHiddenFlag( int bHidden, int bCondition );
    void _SetProtectFlag( int bFlag ) { bProtectFlag = bFlag; }

    int _IsProtect() const;

public:
    TYPEINFO();     // rtti

    SwSection( SectionType eType, const String& rName,
                SwSectionFmt* pFmt = 0 );
    ~SwSection();

    // kopiere nur die Daten der Section!
    // Ableitung bleibt (beim Left) erhalten.
    SwSection& operator=( const SwSection& );
    int operator==( const SwSection& rCmp ) const;

    const String& GetName() const           { return sSectionNm; }
    void SetName( const String& rName )     { sSectionNm = rName; }
    const SectionType GetType() const       { return eType; }
    void SetType( SectionType eNew )        { eType = eNew; }

    SwSectionFmt* GetFmt()          { return (SwSectionFmt*)pRegisteredIn; }
    SwSectionFmt* GetFmt() const    { return (SwSectionFmt*)pRegisteredIn; }

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    // setze die Hidden/Protected -> gesamten Baum updaten !
    // (Attribute/Flags werden gesetzt/erfragt)
    int IsHidden() const { return bHidden; }
    int IsProtect() const { return GetFmt() ? _IsProtect()
                                            : IsProtectFlag(); }

    void SetHidden( int bFlag = TRUE );
    void SetProtect( int bFlag = TRUE );

    // erfrage die internen Flags (Zustand inklusive Parents nicht, was
    // aktuell an der Section gesetzt ist!!)
    int IsHiddenFlag() const { return bHiddenFlag; }
    int IsProtectFlag() const { return bProtectFlag; }

    void SetCondHidden( int bFlag = TRUE );
    int IsCondHidden() const { return bCondHiddenFlag; }
    // erfrage (auch ueber die Parents), ob diese Section versteckt sein soll.
    int CalcHiddenFlag() const;


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
    void SetRefObject( SvPseudoObject* pObj );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }
    BOOL IsServer() const                       {  return refObj.Is(); }

    // Methoden fuer gelinkte Bereiche
    USHORT GetUpdateType() const        { return refLink->GetUpdateMode();  }
    void SetUpdateType( USHORT nType )  { refLink->SetUpdateMode( nType );  }

    BOOL IsConnected() const        { return refLink.Is(); }
    void UpdateNow()                { refLink->Update(); }
    void Disconnect()               { refLink->Disconnect(); }

    const SvBaseLink& GetBaseLink() const   { return *refLink; }

    void CreateLink( LinkCreateType eType );

    void MakeChildLinksVisible( const SwSectionNode& rSectNd );

    BOOL IsLinkType() const
        { return DDE_LINK_SECTION == eType || FILE_LINK_SECTION == eType; }

    // Flags fuer UI - Verbindung geklappt?
    BOOL IsConnectFlag() const                  { return bConnectFlag; }
    void SetConnectFlag( BOOL bFlag = TRUE )    { bConnectFlag = bFlag; }

    // return the TOX base class if the section is a TOX section
    const SwTOXBase* GetTOXBase() const;

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwSection( const SwSection& );
};


enum SectionSort { SORTSECT_NOT, SORTSECT_NAME, SORTSECT_POS };

class SwSectionFmt : public SwFrmFmt
{
    friend class SwDoc;
    SwSection* _GetSection() const;
    void UpdateParent();        // Parent wurde veraendert

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
                            int bAllSections = TRUE ) const;

    // erfrage, ob sich die Section im Nodes-Array oder UndoNodes-Array
    // befindet.
    int IsInNodesArr() const;

          SwSectionNode* GetSectionNode( BOOL bAlways = FALSE );
    const SwSectionNode* GetSectionNode( BOOL bAlways = FALSE ) const
    {   return ((SwSectionFmt*)this)->GetSectionNode( bAlways ); }

    // ist die Section eine gueltige fuers GlobalDocument?
    const SwSection* GetGlobalDocSection() const;
};

// -------------- inlines ---------------------------------

inline SwSection* SwSection::GetParent() const
{
    register SwSectionFmt* pFmt = GetFmt();
    register SwSection* pRet = 0;
    if( pFmt )
        pRet = pFmt->GetParentSection();
    return pRet;
}

inline SwSectionFmt* SwSectionFmt::GetParent() const
{
    register SwSectionFmt* pRet = 0;
    if( GetRegisteredIn() )
        pRet = PTR_CAST( SwSectionFmt, GetRegisteredIn() );
    return pRet;
}

inline SwSection* SwSectionFmt::GetParentSection() const
{
    register SwSectionFmt* pParent = GetParent();
    register SwSection* pRet = 0;
    if( pParent )
        pRet = pParent->_GetSection();
    return pRet;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2001/02/27 18:46:19  jp
      new: Password for single section

      Revision 1.1.1.1  2000/09/18 17:14:27  hr
      initial import

      Revision 1.40  2000/09/18 16:03:26  willem.vandorp
      OpenOffice header added.

      Revision 1.39  2000/09/08 13:24:23  willem.vandorp
      Header and footer replaced

      Revision 1.38  2000/04/19 15:41:58  jp
      Unicode changes

      Revision 1.37  2000/02/11 14:26:27  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.36  1999/11/18 17:52:28  jp
      new: GetTOXBase

      Revision 1.35  1999/07/27 18:20:18  JP
      replace class SwTOXBaseRange with SwTOXBaseSection - TOX use now SwSections


      Rev 1.34   27 Jul 1999 20:20:18   JP
   replace class SwTOXBaseRange with SwTOXBaseSection - TOX use now SwSections

      Rev 1.33   23 Jul 1999 15:53:40   JP
   Bug #67465#: SwSectionFrm CTOR have to change the hiddenflag

      Rev 1.32   21 Jun 1999 10:08:44   JP
   new enums for new TOXs

      Rev 1.31   25 Mar 1999 21:20:34   JP
   Bug #63929#: autom. erzeugen von Tabs im Inhaltsverzeichnis

      Rev 1.30   29 Oct 1998 14:26:18   JP
   Task #58637#: neu Is-/SetConnectFlag

      Rev 1.29   26 Oct 1998 17:29:44   JP
   Bug #54342#: Section merkt sich das Passwort der Datei

      Rev 1.28   12 Jun 1998 14:33:16   JP
   Bug #51097#: im DTOR des SectionFormates ggfs. die ChildLinks sichbar machen

      Rev 1.27   27 Apr 1998 21:16:44   JP
   Set-/SetCondition: Flag fuers Convertieren entfernt

      Rev 1.26   04 Aug 1997 16:43:06   MH
   chg: header

      Rev 1.25   23 Jun 1997 15:44:16   JP
   neu: GetGlobalDocSection

      Rev 1.24   13 Jun 1997 14:54:10   JP
   neu: CalcHiddenFlag

      Rev 1.23   29 Oct 1996 16:17:58   MA
   includes

      Rev 1.22   16 Oct 1996 16:31:22   JP
   unbenutzte Methoden entfernt

      Rev 1.21   10 Jul 1996 17:58:54   OM
   LocalizeDBName in Expfld verschoben

      Rev 1.20   10 Jul 1996 17:49:02   OM
   LocalizeDBName in Expfld verschoben

      Rev 1.19   25 Jun 1996 12:24:14   MA
   headerfiles

      Rev 1.18   11 Jun 1996 16:56:56   JP
   GetLinkFileName: nicht mehr als Inline muessen ggfs. auf den Link zugreifen

      Rev 1.17   24 May 1996 15:42:14   OM
   Bedingungen in Bereichen wandeln

      Rev 1.16   07 Mar 1996 18:11:16   JP
   IsConnect - auf Is() abfragen

      Rev 1.15   12 Feb 1996 18:16:18   JP
   Erweiterung: gelinkte Bereiche

      Rev 1.14   08 Feb 1996 19:14:26   JP
   Erweiterung fuer gelinkte Bereiche

      Rev 1.13   30 Nov 1995 12:54:36   JP
   _SetHiddenFlag - beide Flags ueber geben (Hidden&CondHidden)

      Rev 1.12   08 Nov 1995 10:03:06   OS
   ChangeCondition => SetCondition

      Rev 1.11   11 Jul 1995 18:39:46   JP
   neu: GetSectionNode, Set-/IsCondHidden - Bedingung auswerten

      Rev 1.10   29 Jun 1995 21:11:06   JP
   Section mit Servier-Funktionalitaet

      Rev 1.9   19 Jun 1995 16:45:16   MA
   Umbau fuer daemliche Compiler

      Rev 1.8   14 Jun 1995 16:07:46   JP
   inline vor den inline Methoden

      Rev 1.7   06 Apr 1995 12:42:24   JP
   neu: Member & Methoden fuer den 'gelinkten' Filenamen

      Rev 1.6   23 Feb 1995 17:38:46   JP
   neu: SetName - setze neuen Namen an der Section

      Rev 1.5   03 Feb 1995 18:17:30   JP
   IsProtect jetzt inline

      Rev 1.4   01 Feb 1995 10:52:32   JP
   neu: erfrage/setze Bedingung

      Rev 1.3   26 Jan 1995 18:47:56   JP
   GetChildSection: erweitert fuer UI

      Rev 1.2   26 Jan 1995 11:35:18   JP
   SectionNode als friend der Section - zum Pfegen des HiddenFlags

      Rev 1.1   24 Jan 1995 20:03:12   JP
   neu: IsInNodesArr - abfrage, ob Section nicht im UndoNodes-Array steht

      Rev 1.0   18 Jan 1995 19:22:58   JP
   Initial revision.

*************************************************************************/

#endif
    //_SECTION_HXX

