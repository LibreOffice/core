/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SECTION_HXX
#define _SECTION_HXX

#include <boost/utility.hpp>

#include <com/sun/star/uno/Sequence.h>

#include <tools/rtti.hxx>
#include <tools/ref.hxx>
#include <svl/svarray.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/Metadatable.hxx>

#include <frmfmt.hxx>


namespace com { namespace sun { namespace star {
    namespace text { class XTextSection; }
} } }

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
                    };

enum LinkCreateType
{
    CREATE_NONE,            // nichts weiter tun
    CREATE_CONNECT,         // Link gleich connecten
    CREATE_UPDATE           // Link connecten und updaten
};

class SW_DLLPUBLIC SwSectionData
{
private:
    SectionType m_eType;

    String m_sSectionName;
    String m_sCondition;
    String m_sLinkFileName;
    String m_sLinkFilePassword; // must be changed to Sequence
    ::com::sun::star::uno::Sequence <sal_Int8> m_Password;

    /// it seems this flag caches the current final "hidden" state
    bool m_bHiddenFlag          : 1;
    /// flags that correspond to attributes in the format:
    /// may have different value than format attribute:
    /// format attr has value for this section, while flag is
    /// effectively ORed with parent sections!
    bool m_bProtectFlag         : 1;
    // edit in readonly sections
    bool m_bEditInReadonlyFlag  : 1;

    bool m_bHidden              : 1; // all paragraphs hidden?
    bool m_bCondHiddenFlag      : 1; // Hiddenflag for condition
    bool m_bConnectFlag         : 1; // connected to server?

public:

    SwSectionData(SectionType const eType, String const& rName);
    explicit SwSectionData(SwSection const&);
    SwSectionData(SwSectionData const&);
    SwSectionData & operator=(SwSectionData const&);
    bool operator==(SwSectionData const&) const;

    String const& GetSectionName() const    { return m_sSectionName; }
    void SetSectionName(String const& rName){ m_sSectionName = rName; }
    SectionType GetType() const             { return m_eType; }
    void SetType(SectionType const eNew)    { m_eType = eNew; }

    bool IsHidden() const { return m_bHidden; }
    void SetHidden(bool const bFlag = true) { m_bHidden = bFlag; }

    bool IsHiddenFlag() const { return m_bHiddenFlag; }
    SW_DLLPRIVATE void
        SetHiddenFlag(bool const bFlag) { m_bHiddenFlag = bFlag; }
    bool IsProtectFlag() const { return m_bProtectFlag; }
    SW_DLLPRIVATE void
        SetProtectFlag(bool const bFlag) { m_bProtectFlag = bFlag; }
    bool IsEditInReadonlyFlag() const { return m_bEditInReadonlyFlag; }
    void SetEditInReadonlyFlag(bool const bFlag)
        { m_bEditInReadonlyFlag = bFlag; }

    void SetCondHidden(bool const bFlag = true) { m_bCondHiddenFlag = bFlag; };
    bool IsCondHidden() const { return m_bCondHiddenFlag; }

    String const& GetCondition() const      { return m_sCondition; }
    void SetCondition(String const& rNew)   { m_sCondition = rNew; }

    String const& GetLinkFileName() const   { return m_sLinkFileName; };
    void SetLinkFileName(String const& rNew, String const* pPassWd = 0)
    {
        m_sLinkFileName = rNew;
        if (pPassWd) { SetLinkFilePassword(*pPassWd); }
    }

    String const& GetLinkFilePassword() const   { return m_sLinkFilePassword; }
    void SetLinkFilePassword(String const& rS)  { m_sLinkFilePassword = rS; }

    ::com::sun::star::uno::Sequence<sal_Int8> const& GetPassword() const
                                            { return m_Password; }
    void SetPassword(::com::sun::star::uno::Sequence<sal_Int8> const& rNew)
                                            { m_Password = rNew; }
    bool IsLinkType() const
    { return (DDE_LINK_SECTION == m_eType) || (FILE_LINK_SECTION == m_eType); }

    bool IsConnectFlag() const                  { return m_bConnectFlag; }
    void SetConnectFlag(bool const bFlag = true){ m_bConnectFlag = bFlag; }
};

class SW_DLLPUBLIC SwSection
    : public SwClient
{
    // damit beim Anlegen/Loeschen von Frames das Flag richtig gepflegt wird!
    friend class SwSectionNode;
    // the "read CTOR" of SwSectionFrm have to change the Hiddenflag
    friend class SwSectionFrm;

private:
    SwSectionData m_Data;

    SwServerObjectRef m_RefObj; // set if DataServer
    ::sfx2::SvBaseLinkRef m_RefLink;

    SW_DLLPRIVATE void ImplSetHiddenFlag(
            bool const bHidden, bool const bCondition);

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
    TYPEINFO();     // rtti

    SwSection(SectionType const eType, String const& rName,
                SwSectionFmt & rFormat);
    virtual ~SwSection();

    bool DataEquals(SwSectionData const& rCmp) const;

    void SetSectionData(SwSectionData const& rData);

    String const& GetSectionName() const    { return m_Data.GetSectionName(); }
    void SetSectionName(String const& rName){ m_Data.SetSectionName(rName); }
    SectionType GetType() const             { return m_Data.GetType(); }
    void SetType(SectionType const eType)   { return m_Data.SetType(eType); }

    SwSectionFmt* GetFmt()          { return (SwSectionFmt*)GetRegisteredIn(); }
    SwSectionFmt* GetFmt() const    { return (SwSectionFmt*)GetRegisteredIn(); }

    // setze die Hidden/Protected -> gesamten Baum updaten !
    // (Attribute/Flags werden gesetzt/erfragt)
    bool IsHidden()  const { return m_Data.IsHidden(); }
    void SetHidden (bool const bFlag = true);
    bool IsProtect() const;
    void SetProtect(bool const bFlag = true);
    bool IsEditInReadonly() const;
    void SetEditInReadonly(bool const bFlag = true);

    // erfrage die internen Flags (Zustand inklusive Parents nicht, was
    // aktuell an der Section gesetzt ist!!)
    bool IsHiddenFlag()  const { return m_Data.IsHiddenFlag(); }
    bool IsProtectFlag() const { return m_Data.IsProtectFlag(); }
    bool IsEditInReadonlyFlag() const { return m_Data.IsEditInReadonlyFlag(); }

    void SetCondHidden(bool const bFlag = true);
    bool IsCondHidden() const { return m_Data.IsCondHidden(); }
    // erfrage (auch ueber die Parents), ob diese Section versteckt sein soll.
    sal_Bool CalcHiddenFlag() const;


    inline SwSection* GetParent() const;

    String const& GetCondition() const      { return m_Data.GetCondition(); }
    void SetCondition(String const& rNew)   { m_Data.SetCondition(rNew); }

    const String& GetLinkFileName() const;
    void SetLinkFileName(String const& rNew, String const*const pPassWd = 0);
    // password of linked file (only valid during runtime!)
    String const& GetLinkFilePassword() const
        { return m_Data.GetLinkFilePassword(); }
    void SetLinkFilePassword(String const& rS)
        { m_Data.SetLinkFilePassword(rS); }

    // get / set password of this section
    ::com::sun::star::uno::Sequence<sal_Int8> const& GetPassword() const
                                            { return m_Data.GetPassword(); }
    void SetPassword(::com::sun::star::uno::Sequence <sal_Int8> const& rNew)
                                            { m_Data.SetPassword(rNew); }

    // Daten Server-Methoden
    void SetRefObject( SwServerObject* pObj );
    const SwServerObject* GetObject() const {  return & m_RefObj; }
          SwServerObject* GetObject()       {  return & m_RefObj; }
    bool IsServer() const                   {  return m_RefObj.Is(); }

    // Methoden fuer gelinkte Bereiche
    sal_uInt16 GetUpdateType() const    { return m_RefLink->GetUpdateMode(); }
    void SetUpdateType(sal_uInt16 const nType )
        { m_RefLink->SetUpdateMode(nType); }

    bool IsConnected() const        { return m_RefLink.Is(); }
    void UpdateNow()                { m_RefLink->Update(); }
    void Disconnect()               { m_RefLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *m_RefLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *m_RefLink; }

    void CreateLink( LinkCreateType eType );

    void MakeChildLinksVisible( const SwSectionNode& rSectNd );

    bool IsLinkType() const { return m_Data.IsLinkType(); }

    // Flags fuer UI - Verbindung geklappt?
    bool IsConnectFlag() const      { return m_Data.IsConnectFlag(); }
    void SetConnectFlag(bool const bFlag = true)
                                    { m_Data.SetConnectFlag(bFlag); }

    // return the TOX base class if the section is a TOX section
    const SwTOXBase* GetTOXBase() const;

    void BreakLink();

};


enum SectionSort { SORTSECT_NOT, SORTSECT_NAME, SORTSECT_POS };

class SW_DLLPUBLIC SwSectionFmt
    : public SwFrmFmt
    , public ::sfx2::Metadatable
{
    friend class SwDoc;

    /** why does this exist in addition to the m_wXObject in SwFrmFmt?
        in case of an index, both a SwXDocumentIndex and a SwXTextSection
        register at this SwSectionFmt, so we need to have two refs.
     */
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextSection> m_wXTextSection;

    SW_DLLPRIVATE void UpdateParent();      // Parent wurde veraendert

protected:
    SwSectionFmt( SwSectionFmt* pDrvdFrm, SwDoc *pDoc );
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
    ~SwSectionFmt();

    //Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
    virtual void DelFrms();

    //Erzeugt die Ansichten
    virtual void MakeFrms();

        // erfrage vom Format Informationen
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    SwSection* GetSection() const;
    inline SwSectionFmt* GetParent() const;
    inline SwSection* GetParentSection() const;

    // alle Sections, die von dieser abgeleitet sind
    //  - sortiert nach : Name oder Position oder unsortiert
    //  - alle oder nur die, die sich im normalten Nodes-Array befinden
    sal_uInt16 GetChildSections( SwSections& rArr,
                            SectionSort eSort = SORTSECT_NOT,
                            sal_Bool bAllSections = sal_True ) const;

    // erfrage, ob sich die Section im Nodes-Array oder UndoNodes-Array
    // befindet.
    sal_Bool IsInNodesArr() const;

          SwSectionNode* GetSectionNode(bool const bEvenIfInUndo = false);
    const SwSectionNode* GetSectionNode(bool const bEvenIfInUndo = false) const
        { return const_cast<SwSectionFmt *>(this)
                ->GetSectionNode(bEvenIfInUndo); }

    // ist die Section eine gueltige fuers GlobalDocument?
    const SwSection* GetGlobalDocSection() const;

    SW_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextSection> const& GetXTextSection() const
            { return m_wXTextSection; }
    SW_DLLPRIVATE void SetXTextSection(::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextSection> const& xTextSection)
            { m_wXTextSection = xTextSection; }

    // sfx2::Metadatable
    virtual ::sfx2::IXmlIdRegistry& GetRegistry();
    virtual bool IsInClipboard() const;
    virtual bool IsInUndo() const;
    virtual bool IsInContent() const;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject();

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
    {
        pRet = pParent->GetSection();
    }
    return pRet;
}


#endif /* _SECTION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
