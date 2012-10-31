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


#include <vector>
#include <map>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <sot/storage.hxx>
#include <svtools/svtools.hrc>

#include <sal/main.h>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/filectrl.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <vcl/unohelp2.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/svmedit.hxx>
#include <sfx2/filedlghelper.hxx>

#include <toolkit/unohlp.hxx>

#include <tools/stream.hxx>
#include <tools/resmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <svx/msdffdef.hxx>

#include <unotools/localfilehelper.hxx>

#include "xmlconfig.hxx"

using ::rtl::OUString;

using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////

enum CompareStatus           { CMP_NOTYET = 0, CMP_EQUAL = 1, CMP_NOTEQUAL = 2, CMP_NOTAVAILABLE = 3 };
static ColorData gColors[] = { COL_BLACK,      COL_GREEN,     COL_RED,          COL_CYAN };

class Atom
{
public:
    ~Atom();

    /** imports this atom and its child atoms */
    static Atom* import( const DffRecordHeader& rRootRecordHeader, SvStream& rStCtrl );
    static Atom* import( UINT16 nRecType, SvStream& rStCtrl );

    inline const DffRecordHeader& getHeader() const;

    /** returns true if at least one atim with the given nRecType is found */
    inline bool hasChildAtom( sal_uInt16 nRecType ) const;

    /** returns true if at least one atim with the given nRecType and nRecInstnace is found */
    inline bool hasChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const;

    /** returns the first child atom with nRecType or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType ) const;

    /** returns the next child atom after pLast with nRecType or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const;

    /** returns the first child atom with nRecType and nRecInstance or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const;

    /** returns the next child atom after pLast with nRecType and nRecInstance or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance, const Atom* pLast ) const;

    /** returns the first child atom or NULL */
    inline const Atom* findFirstChildAtom() const;

    /** returns the next child atom after pLast or NULL */
    inline const Atom* findNextChildAtom( const Atom* pLast ) const;

    /** returns true if this atom is a container */
    inline bool isContainer() const;

    /** seeks to the contents of this atom */
    inline bool seekToContent() const;

    /** returns the record type */
    inline sal_uInt16 getType() const;

    /** returns the record instance */
    inline sal_uInt16 getInstance() const;

    /** returns the record length */
    inline sal_uInt32 getLength() const;

    SvStream& getStream() const { return mrStream; }

    bool operator==( const Atom& rAtom ) const;

    CompareStatus getCompareStatus() const { return meStatus; }

    void compare( Atom* pAtom );
    bool compareContent( Atom& rAtom );

    Atom* getCompareAtom() const { return mpCompareAtom; }
    void setCompareAtom( Atom* pAtom ) { mpCompareAtom = pAtom; }

private:
    Atom( const DffRecordHeader& rRecordHeader, SvStream& rStCtrl );

    // statics for compare
    static Atom* skipAtoms( Atom* pContainer, Atom* pAtom, Atom* pSkipTo );
    static Atom* findFirstEqualAtom( Atom* pCompare, Atom* pContainer, Atom* pSearch, int& nDistance );

    SvStream& mrStream;
    DffRecordHeader maRecordHeader;
    Atom* mpFirstChild;
    Atom* mpNextAtom;

    CompareStatus meStatus;
    Atom* mpCompareAtom;
};

bool Atom::operator==( const Atom& rAtom ) const
{
    return ( maRecordHeader.nRecType == rAtom.maRecordHeader.nRecType ) &&
            ( maRecordHeader.nRecVer == rAtom.maRecordHeader.nRecVer ) &&
           ( maRecordHeader.nRecInstance == rAtom.maRecordHeader.nRecInstance );
}

bool Atom::compareContent( Atom& rAtom )
{
    if( maRecordHeader.nRecLen == rAtom.maRecordHeader.nRecLen )
    {
        seekToContent();
        rAtom.seekToContent();

        SvStream& rStream1 = getStream();
        SvStream& rStream2 = rAtom.getStream();

        const int nBufferSize = 1024;
        boost::shared_ptr< char > buffer1( new char[nBufferSize] );
        boost::shared_ptr< char > buffer2( new char[nBufferSize] );

        sal_uInt32 nLength = maRecordHeader.nRecLen;
        sal_Size nRead = 0;
        while( nLength )
        {
            sal_Size nRead = (nBufferSize < nLength) ? nBufferSize : nLength;
            nRead = rStream1.Read( (void*)buffer1.get(), nRead );
            if( nRead == 0 )
                break;
            if( rStream2.Read( (void*)buffer2.get(), nRead ) != nRead )
                break;
            if( memcmp( (void*)buffer1.get(), (void*)buffer2.get(), nRead ) != 0 )
                break;

            nLength -= nRead;
        }

        return nLength == 0;
    }

    return false;
}

inline bool Atom::hasChildAtom( sal_uInt16 nRecType ) const
{
    return findFirstChildAtom( nRecType ) != NULL;
}

inline bool Atom::hasChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const
{
    return findFirstChildAtom( nRecType, nRecInstance ) != NULL;
}

inline const Atom* Atom::findFirstChildAtom( sal_uInt16 nRecType ) const
{
    return findNextChildAtom( nRecType, NULL );
}

inline const DffRecordHeader& Atom::getHeader() const
{
    return maRecordHeader;
}

inline const Atom* Atom::findFirstChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const
{
    return findNextChildAtom( nRecType, nRecInstance, NULL );
}

inline const Atom* Atom::findFirstChildAtom() const
{
    return mpFirstChild;
}

inline const Atom* Atom::findNextChildAtom( const Atom* pLast ) const
{
    return pLast ? pLast->mpNextAtom : pLast;
}

inline bool Atom::isContainer() const
{
    return (bool)maRecordHeader.IsContainer();
}

inline bool Atom::seekToContent() const
{
    maRecordHeader.SeekToContent( mrStream );
    return mrStream.GetError() == 0;
}

inline sal_uInt16 Atom::getType() const
{
    return maRecordHeader.nRecType;
}

inline sal_uInt16 Atom::getInstance() const
{
    return maRecordHeader.nRecInstance;
}

inline sal_uInt32 Atom::getLength() const
{
    return maRecordHeader.nRecLen;
}

Atom::Atom( const DffRecordHeader& rRecordHeader, SvStream& rStream )
: maRecordHeader( rRecordHeader ),
  mrStream( rStream ),
  mpFirstChild( 0 ),
  mpNextAtom( 0 ),
  meStatus( CMP_NOTYET ),
  mpCompareAtom( 0 )
{
    // check if we need to force this to a container
    if( maRecordHeader.nRecVer != DFF_PSFLAG_CONTAINER )
    {
        AtomConfig* pAtomConfig = dynamic_cast< AtomConfig* >( gAtomConfigMap[ maRecordHeader.nRecType ].get() );
        if( pAtomConfig && pAtomConfig->isContainer() )
        {
            maRecordHeader.nRecVer = DFF_PSFLAG_CONTAINER;
        }
    }

    if( isContainer() )
    {
        if( seekToContent() )
        {
            DffRecordHeader aChildHeader;

            Atom* pLastAtom = NULL;

            while( (mrStream.GetError() == 0 ) && ( mrStream.Tell() < maRecordHeader.GetRecEndFilePos() ) )
            {
                mrStream >> aChildHeader;

                if( mrStream.GetError() == 0 )
                {
                    Atom* pAtom = new Atom( aChildHeader, mrStream );

                    if( pLastAtom )
                        pLastAtom->mpNextAtom = pAtom;
                    if( mpFirstChild == NULL )
                        mpFirstChild = pAtom;

                    pLastAtom = pAtom;
                }
            }
        }
    }

    maRecordHeader.SeekToEndOfRecord( mrStream );
}

Atom::~Atom()
{
    Atom* pChild = mpFirstChild;
    while( pChild )
    {
        Atom* pNextChild = pChild->mpNextAtom;
        delete pChild;
        pChild = pNextChild;
    }
}

/** imports this atom and its child atoms */
Atom* Atom::import( const DffRecordHeader& rRootRecordHeader, SvStream& rStCtrl )
{
    Atom* pRootAtom = new Atom( rRootRecordHeader, rStCtrl );

    if( rStCtrl.GetError() == 0 )
    {
        return pRootAtom;
    }
    else
    {
        delete pRootAtom;
        return NULL;
    }
}

/** imports this atom and its child atoms */
Atom* Atom::import( UINT16 nRecType, SvStream& rStCtrl )
{
    rStCtrl.Seek( STREAM_SEEK_TO_END );
    sal_Size nStreamLength = rStCtrl.Tell();
    rStCtrl.Seek( STREAM_SEEK_TO_BEGIN );

    DffRecordHeader aRootRecordHeader;
    aRootRecordHeader.nRecVer = DFF_PSFLAG_CONTAINER;
    aRootRecordHeader.nRecInstance = 0;
    aRootRecordHeader.nImpVerInst = 0;
    aRootRecordHeader.nRecType = nRecType;
    aRootRecordHeader.nRecLen = nStreamLength;
    aRootRecordHeader.nFilePos = 0;

    return import( aRootRecordHeader, rStCtrl );
}

/** returns the next child atom after pLast with nRecType or NULL */
const Atom* Atom::findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const
{
    Atom* pChild = pLast != NULL ? pLast->mpNextAtom : mpFirstChild;
    while( pChild && pChild->maRecordHeader.nRecType != nRecType )
    {
        pChild = pChild->mpNextAtom;
    }

    return pChild;
}

/** returns the next child atom after pLast with nRecType and nRecInstance or NULL */
const Atom* Atom::findNextChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance, const Atom* pLast ) const
{
    const Atom* pChild = pLast != NULL ? pLast->mpNextAtom : mpFirstChild;
    while( pChild && (pChild->maRecordHeader.nRecType != nRecType) && (pChild->maRecordHeader.nRecInstance != nRecInstance) )
    {
        pChild = findNextChildAtom( pChild );
    }

    return pChild;
}

Atom* Atom::findFirstEqualAtom( Atom* pCompare, Atom* pContainer, Atom* pSearch, int& nDistance )
{
    nDistance = 0;

    while( pSearch )
    {
        if( *pSearch == *pCompare )
            return pSearch;

        pSearch = const_cast< Atom* >( pContainer->findNextChildAtom( pSearch ) );
        nDistance++;
    }

    return 0;
}

Atom* Atom::skipAtoms( Atom* pContainer, Atom* pAtom, Atom* pSkipTo )
{
    while( pAtom && (pAtom != pSkipTo) )
    {
        pAtom->meStatus = CMP_NOTAVAILABLE;
        pAtom = const_cast< Atom* >( pContainer->findNextChildAtom( pAtom ) );
    }

    return pAtom;
}

void Atom::compare( Atom* pAtom )
{
    if( pAtom )
    {
        if( meStatus == CMP_NOTYET )
        {
            mpCompareAtom = pAtom;
            pAtom->mpCompareAtom = this;

            mpCompareAtom = pAtom;
            pAtom->mpCompareAtom = this;

            meStatus = pAtom->meStatus = ( *this == *pAtom ) ? CMP_EQUAL : CMP_NOTEQUAL;
        }

        if(meStatus == CMP_EQUAL)
        {
            if( isContainer() )
            {
                /** returns the first child atom or NULL */
                Atom* pChildAtom1 = const_cast< Atom* >( findFirstChildAtom() );

                if( pChildAtom1 && (pChildAtom1->meStatus == CMP_NOTYET) )
                {
                    Atom* pChildAtom2 = const_cast< Atom* >( pAtom->findFirstChildAtom() );
                    while( pChildAtom1 && pChildAtom2 )
                    {
                        if( !(*pChildAtom1 == *pChildAtom2) )
                        {
                            int nDistance1;
                            int nDistance2;

                            Atom* pFind1 = findFirstEqualAtom( pChildAtom1, pAtom, const_cast< Atom* >( pAtom->findNextChildAtom( pChildAtom2 )), nDistance1 );
                            Atom* pFind2 = findFirstEqualAtom( pChildAtom2, this, const_cast< Atom* >(findNextChildAtom( pChildAtom1 )), nDistance2 );

                            if( pFind1 && (!pFind2 || (nDistance1 < nDistance2) ) )
                            {
                                pChildAtom2 = skipAtoms( pAtom, pChildAtom2, pFind1 );
                            }
                            else if( pFind2 )
                            {
                                pChildAtom1 = skipAtoms( this, pChildAtom1, pFind2 );
                            }
                            else
                            {
                                pChildAtom1 = skipAtoms( this, pChildAtom1, 0 );
                                pChildAtom2 = skipAtoms( pAtom, pChildAtom2, 0 );
                            }
                        }

                        if( pChildAtom1 && pChildAtom2 )
                        {
                            pChildAtom1->mpCompareAtom = pChildAtom2;
                            pChildAtom2->mpCompareAtom = pChildAtom1;

                            pChildAtom1->meStatus = pChildAtom2->meStatus =
                                (pChildAtom1->isContainer() || pChildAtom1->compareContent( *pChildAtom2 )) ?
                                    CMP_EQUAL : CMP_NOTEQUAL;

                            pChildAtom1 = const_cast< Atom* >( findNextChildAtom( pChildAtom1 ) );
                            pChildAtom2 = const_cast< Atom* >( pAtom->findNextChildAtom( pChildAtom2 ) );
                        }
                    }
                }
            }
            else
            {
                if( !compareContent( *pAtom ) )
                {
                    meStatus = pAtom->meStatus = CMP_NOTEQUAL;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

class AtomBoxString : public SvLBoxString
{
public:
    AtomBoxString( SvTreeListEntry* pEntry, const String& rStr )
        : SvLBoxString( pEntry, 0, rStr )
    { }

    ~AtomBoxString() { }

    void Paint( const Point& rPos, SvLBox& rOutDev, USHORT nViewDataEntryFlags, SvTreeListEntry* pEntry )
    {
        Color aOldTextColor = rOutDev.GetTextColor();

        if( pEntry && pEntry->GetUserData() )
        {
            Atom* pAtom = static_cast<Atom*>( pEntry->GetUserData() );
            rOutDev.SetTextColor( Color( gColors[ pAtom->getCompareStatus() ] ) );
        }

        SvLBoxString::Paint( rPos, rOutDev, nViewDataEntryFlags, pEntry );

        rOutDev.SetTextColor( aOldTextColor );
    }
};


//////////////////////////////////////////////////////////////////////

class AtomContainerTreeListBox : public SvTreeListBox
{
public:
    AtomContainerTreeListBox( Window* pParent );
    ~AtomContainerTreeListBox();

    void SetRootAtom( const Atom* pAtom );


    void            SetCollapsingHdl(const Link& rNewHdl){maCollapsingHdl=rNewHdl;}
    const Link&     GetCollapsingHdl() const { return maCollapsingHdl; }

    void            SetExpandingHdl(const Link& rNewHdl){maExpandingHdl=rNewHdl;}
    const Link&     GetExpandingHdl() const { return maExpandingHdl; }

    virtual BOOL    Expand( SvTreeListEntry* pParent );
    virtual BOOL    Collapse( SvTreeListEntry* pParent );

    SvTreeListEntry*    findAtom( Atom* pAtom );

    virtual void InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&);
    virtual void SetTabs();

private:
    void InsertAtom( const Atom* pAtom, SvTreeListEntry* pParent = 0 );
    const Atom* mpRootAtom;
    ResMgr* mpResMgr;
    Image maImgFolder;
    Image maImgAtom;
    Image maImgExpanded;
    Image maImgCollapsed;
    bool mbRecursiveGuard;
    Link maCollapsingHdl;
    Link maExpandingHdl;
};

typedef std::pair< AtomContainerTreeListBox*, SvTreeListEntry* > AtomContainerEntryPair;

AtomContainerTreeListBox::AtomContainerTreeListBox( Window* pParent )
: SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_HASBUTTONSATROOT|WB_3DLOOK|WB_BORDER ),
    mpRootAtom( 0 ), mbRecursiveGuard( false )
{
    mpResMgr = ResMgr::CreateResMgr( "svt" );
    maImgCollapsed = Image( ResId( RID_IMG_TREENODE_COLLAPSED, mpResMgr ) );
    maImgExpanded = Image( ResId( RID_IMG_TREENODE_EXPANDED, mpResMgr ) );

//  SetDefaultExpandedEntryBmp( aExpanded );
//  SetDefaultCollapsedEntryBmp(aCollapsed );

    maImgFolder = Image( ResId( IMG_SVT_FOLDER, mpResMgr ) );
    maImgAtom = Image( ResId( IMG_SVT_DOCTEMPLATE_DOCINFO_SMALL, mpResMgr ) );
}

AtomContainerTreeListBox::~AtomContainerTreeListBox()
{
}

void AtomContainerTreeListBox::SetTabs()
{
    if( IsEditingActive() )
        EndEditing( TRUE );

    ClearTabList();

    short nIndent = 0; GetIndent();
    long nNodeWidthPixel = maImgCollapsed.GetSizePixel().Width();
    long nContextWidthDIV2 = nNodeWidthPixel >> 1;

    long nStartPos = 2 + ( nIndent + nContextWidthDIV2 );
    AddTab( nStartPos, SV_LBOXTAB_DYNAMIC | SV_LBOXTAB_ADJUST_CENTER );
    nStartPos += nNodeWidthPixel + 5;
    AddTab( nStartPos, SV_LBOXTAB_DYNAMIC | SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_SHOW_SELECTION );
    nStartPos += nContextWidthDIV2 + 5;
    AddTab( nStartPos, SV_LBOXTAB_DYNAMIC|SV_LBOXTAB_ADJUST_LEFT | SV_LBOXTAB_SHOW_SELECTION );
}

void AtomContainerTreeListBox::InitEntry(SvTreeListEntry* pEntry, const OUString& aStr,
    const Image& aCollEntryBmp, const Image& aExpEntryBmp)
{
    pEntry->AddItem( new SvLBoxContextBmp( pEntry,0, aCollEntryBmp,aExpEntryBmp, SVLISTENTRYFLAG_EXPANDED ) );
    pEntry->AddItem( new SvLBoxContextBmp( pEntry,0, maImgAtom, maImgAtom, SVLISTENTRYFLAG_EXPANDED ) );
    pEntry->AddItem( new AtomBoxString( pEntry, aStr ) );
}

SvTreeListEntry* AtomContainerTreeListBox::findAtom( Atom* pAtom )
{
    SvTreeListEntry* pEntry = First();
    while( pEntry )
    {
        if( pEntry->GetUserData() == pAtom )
            return pEntry;

        pEntry = Next( pEntry );
    }

    return 0;
}

BOOL AtomContainerTreeListBox::Expand( SvTreeListEntry* pParent )
{
    BOOL bRet = FALSE;
    if( !mbRecursiveGuard )
    {
        mbRecursiveGuard = true;
        AtomContainerEntryPair aPair( this, pParent );
        maExpandingHdl.Call( &aPair);

        bRet = SvTreeListBox::Expand( pParent );
        mbRecursiveGuard = false;
    }
    return bRet;
}

BOOL AtomContainerTreeListBox::Collapse( SvTreeListEntry* pParent )
{
    BOOL bRet = FALSE;
    if( !mbRecursiveGuard )
    {
        mbRecursiveGuard = true;
        AtomContainerEntryPair aPair( this, pParent );
        maCollapsingHdl.Call( &aPair);

        bRet = SvTreeListBox::Collapse( pParent );
        mbRecursiveGuard = false;
    }
    return bRet;
}

void AtomContainerTreeListBox::SetRootAtom( const Atom* pAtom )
{
    mpRootAtom = pAtom;
    InsertAtom( mpRootAtom );
}

void AtomContainerTreeListBox::InsertAtom( const Atom* pAtom, SvTreeListEntry* pParent /* = 0 */ )
{
    if( pAtom )
    {
        const DffRecordHeader& rHeader = pAtom->getHeader();

        char buffer[1024];

        rtl::OUString aText;
        AtomConfig* pAtomConfig = dynamic_cast< AtomConfig*>( gAtomConfigMap[rHeader.nRecType].get() );

        if( pAtomConfig )
            aText = pAtomConfig->getName();

        if( !aText.getLength() )
        {
            sprintf( buffer, "unknown_0x%04x", rHeader.nRecType );
            aText += rtl::OUString::createFromAscii( buffer );
        }

        sprintf( buffer, " (I: %lu L: %lu)", (UINT32)rHeader.nRecVer, (UINT32)rHeader.nRecLen );
        aText += String( rtl::OUString::createFromAscii( buffer ) );

        SvTreeListEntry* pEntry = 0;
        if( pAtom->isContainer() && pAtom->findFirstChildAtom() )
        {
            pEntry = InsertEntry( aText, maImgExpanded, maImgCollapsed, pParent );

            /** returns the first child atom or NULL */
            const Atom* pChildAtom = pAtom->findFirstChildAtom();

            while( pChildAtom )
            {
                InsertAtom( pChildAtom, pEntry );
                pChildAtom = pAtom->findNextChildAtom( pChildAtom );
            }
        }
        else
        {
            pEntry = InsertEntry( aText, pParent );
        }

        if( pEntry )
        {
            pEntry->SetUserData( (void*)pAtom );

            if( pAtom->isContainer() )
            {
                SvLBoxContextBmp* pBoxBmp = dynamic_cast< SvLBoxContextBmp* >( pEntry->GetItem( pEntry->ItemCount() - 2 ) );
                if( pBoxBmp )
                {
                    pBoxBmp->SetBitmap1( pEntry, maImgFolder );
                    pBoxBmp->SetBitmap2( pEntry, maImgFolder );
                }
            }

/*
            pEntry->ReplaceItem(
                new AtomBoxString( pEntry, aText, pImage ),
                pEntry->ItemCount() - 1 );
*/
        }
    }
}

///////////////////////////////////////////////////////////////////////

extern void load_config( const OUString& rPath );

class PPTDocument
{
public:
    PPTDocument( const rtl::OUString& rFilePath );
    ~PPTDocument();

    Atom* getRootAtom() const;

private:
    void Load( const rtl::OUString& rFilePath );

    Atom* mpAtom;
    SvStream* mpDocStream;
    SotStorageRef maStorage;
};

typedef boost::shared_ptr< PPTDocument > PPTDocumentPtr;

PPTDocument::PPTDocument(const rtl::OUString& rFilePath)
: mpAtom(0), mpDocStream(0)
{
    Load( rFilePath );
}

PPTDocument::~PPTDocument()
{
    delete mpAtom;
    delete mpDocStream;
}

void PPTDocument::Load( const rtl::OUString& rFilePath )
{
    maStorage = new SotStorage( rFilePath, STREAM_STD_READ );
    if( !maStorage->GetError() )
    {
        mpDocStream = maStorage->OpenSotStream( String( "PowerPoint Document" ), STREAM_STD_READ );
        if( mpDocStream )
        {
            DffRecordHeader aRecordHeader;
            *mpDocStream >> aRecordHeader;

            mpAtom = Atom::import( 65530, *mpDocStream );
        }
    }
}

Atom* PPTDocument::getRootAtom() const
{
    return mpAtom;
}

///////////////////////////////////////////////////////////////////////

class MSViewerWorkWindow : public WorkWindow
{
public:
    MSViewerWorkWindow();
    ~MSViewerWorkWindow();

    PPTDocumentPtr Load();
    void onView();
    void onCompare();
    void onClose();

    void View( const PPTDocumentPtr& pDocument, int nPane );
    void Compare( const PPTDocumentPtr& pDocument1, const PPTDocumentPtr& pDocument2 );

    virtual void Resize();

private:
    void Sync( AtomContainerEntryPair* pPair, int nAction );

    AtomContainerTreeListBox*   mpListBox[2];
    MultiLineEdit*              mpEdit[2];
    PPTDocumentPtr              mpDocument[2];
    MenuBar*                    mpMenuBar;
    PopupMenu*                  mpFileMenu;
    bool mbSelectHdlGuard;
    DECL_LINK( implSelectHdl, AtomContainerTreeListBox* );
    DECL_LINK( implExpandingHdl, AtomContainerEntryPair* );
    DECL_LINK( implCollapsingHdl, AtomContainerEntryPair* );
    DECL_LINK( implMenuHdl, Menu* );
};

// -----------------------------------------------------------------------

void MSViewerWorkWindow::onView()
{
    PPTDocumentPtr pDocument( Load() );
    if( pDocument.get() )
    {
        onClose();
        View( pDocument, 0 );
    }
}

void MSViewerWorkWindow::onClose()
{
}

void MSViewerWorkWindow::onCompare()
{
    PPTDocumentPtr pDocument1( Load() );
    if( pDocument1.get() )
    {
        PPTDocumentPtr pDocument2( Load() );
        if( pDocument2.get() )
        {
            onClose();
            Compare( pDocument1, pDocument2 );
        }
    }
}

void MSViewerWorkWindow::Compare( const PPTDocumentPtr& pDocument1, const PPTDocumentPtr& pDocument2 )
{
    if( pDocument1.get() && pDocument2.get() )
    {
        Atom* pAtom1 = pDocument1->getRootAtom();
        Atom* pAtom2 = pDocument2->getRootAtom();
        pAtom1->setCompareAtom( pAtom2 );
        pAtom2->setCompareAtom( pAtom1 );
    }

    View( pDocument1, 0 );
    View( pDocument2, 1 );
}

void MSViewerWorkWindow::View( const PPTDocumentPtr& pDocument, int nPane )
{
    if( ((nPane != 0) && (nPane != 1)) || (pDocument.get() == 0) )
        return;

    mpDocument[nPane] = pDocument;

    mpListBox[nPane]->SetRootAtom( pDocument->getRootAtom() );
    mpListBox[nPane]->Expand( mpListBox[nPane]->GetEntry(0) );
    mpListBox[nPane]->Show();
    mpEdit[nPane]->Show();
    Resize();
}


PPTDocumentPtr MSViewerWorkWindow::Load()
{
    ::sfx2::FileDialogHelper aDlg(
        ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    String aStrFilterType(  "*.ppt"  );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );
//  INetURLObject aFile( SvtPathOptions().GetPalettePath() );
//  aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

    PPTDocumentPtr pDocument;
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        pDocument.reset( new PPTDocument( aDlg.GetPath() ) );
    }

    return pDocument;
}

// -----------------------------------------------------------------------

MSViewerWorkWindow::MSViewerWorkWindow() :
    WorkWindow( 0, WB_APP | WB_STDWORK | WB_3DLOOK ),mbSelectHdlGuard(false)
{
    Size aOutputSize( 400, 600 );
    SetOutputSizePixel( aOutputSize );
    SetText( String(  "MSViewer"  ) );

    Size aOutSize( GetOutputSizePixel() );

    Font aFont( String(  "Courier"  ), GetFont().GetSize() );

    mpMenuBar = new MenuBar();
    mpMenuBar->InsertItem( 1, String( "~File"  ) );
    mpFileMenu = new PopupMenu();
    mpFileMenu->InsertItem( 2, String( "~View"  ) );
    mpFileMenu->InsertItem( 3, String( "~Compare"  ) );
    mpFileMenu->InsertSeparator();
    mpFileMenu->InsertItem( 4, String( "~Quit"  ) );
    mpFileMenu->SetSelectHdl( LINK( this, MSViewerWorkWindow, implMenuHdl ) );

    mpMenuBar->SetPopupMenu( 1, mpFileMenu );
    SetMenuBar( mpMenuBar );
    int nPane;
    for( nPane = 0; nPane < 2; nPane++ )
    {
        mpListBox[nPane] = new AtomContainerTreeListBox( this );
        mpListBox[nPane]->SetSelectHdl( LINK( this, MSViewerWorkWindow, implSelectHdl ) );
        mpListBox[nPane]->SetExpandingHdl( LINK( this, MSViewerWorkWindow, implExpandingHdl ) );
        mpListBox[nPane]->SetCollapsingHdl( LINK( this, MSViewerWorkWindow, implCollapsingHdl ) );

        mpEdit[nPane] = new MultiLineEdit(this, WB_3DLOOK | WB_BORDER | WB_LEFT | WB_TOP | WB_READONLY | WB_HSCROLL | WB_VSCROLL );
        mpEdit[nPane]->SetReadOnly( TRUE );
        mpEdit[nPane]->SetReadOnly( TRUE );
        mpEdit[nPane]->SetControlFont( aFont );
    }
}

// -----------------------------------------------------------------------

static String GetAtomText( const Atom* pAtom )
{
    String aText;
    if( pAtom )
    {
        const DffRecordHeader& rHeader = pAtom->getHeader();
        char buffer[512];
        sprintf( buffer, "Version = %lu\n\rInstance = %lu\n\rVersionInstance = %lu\n\rLength = %lu\n\r",
        (UINT32)rHeader.nRecVer,
        (UINT32)rHeader.nRecInstance,
        (UINT32)rHeader.nImpVerInst,
        (UINT32)rHeader.nRecLen );
        aText = rtl::OUString::createFromAscii( buffer );
        if( pAtom->isContainer() )
        {

        }
        else
        {
            pAtom->seekToContent();
            AtomConfig* pAtomConfig = dynamic_cast< AtomConfig* >( gAtomConfigMap[pAtom->getType()].get() );
            if( pAtomConfig )
            {
                sal_Size nLength = pAtom->getLength();
                aText += String( pAtomConfig->format( pAtom->getStream(), nLength ) );
            }
            else
            {
                sal_Size nLength = pAtom->getLength();
                aText += String( ElementConfig::dump_hex( pAtom->getStream(), nLength ) );
            }
        }
    }

    return aText;
}

IMPL_LINK(MSViewerWorkWindow,implSelectHdl, AtomContainerTreeListBox*, pListBox )
{
    int nPane = (pListBox == mpListBox[1]) ? 1 : 0;
    SvTreeListEntry* pEntry = mpListBox[nPane]->FirstSelected();
    if( pEntry && pEntry->GetUserData() )
    {
        Atom* pAtom = static_cast<Atom*>( pEntry->GetUserData() );
        mpEdit[nPane]->SetText( GetAtomText( pAtom ) );

        if(!mbSelectHdlGuard)
        {
            mbSelectHdlGuard = true;
            // select other
            AtomContainerEntryPair aPair( pListBox, pEntry );
            Sync( &aPair, 2 );
            mbSelectHdlGuard = false;
        }
    }
    return 0;
}

void MSViewerWorkWindow::Sync( AtomContainerEntryPair* pPair, int nAction )
{
    if( mpDocument[0].get() && mpDocument[1].get() && pPair->first && pPair->second )
    {
        AtomContainerTreeListBox* pDestinationListBox = (pPair->first == mpListBox[0]) ? mpListBox[1] : mpListBox[0];

        Atom* pAtom = static_cast<Atom*>(pPair->second->GetUserData());
        if( pAtom && pAtom->getCompareAtom() )
        {
            SvTreeListEntry* pEntry = pDestinationListBox->findAtom( pAtom->getCompareAtom() );

            if(pEntry )
            {
                if( nAction == 0 )
                {
                    pDestinationListBox->Expand( pEntry );
                }
                else if( nAction == 1 )
                {
                    pDestinationListBox->Collapse( pEntry );
                }
                else
                {
                    pDestinationListBox->Select( pEntry );
                }
            }
        }
    }
}

IMPL_LINK(MSViewerWorkWindow, implExpandingHdl, AtomContainerEntryPair*, pPair )
{
    SvTreeListEntry* pEntry = pPair->second;
    if( pEntry && pEntry->GetUserData() )
    {
        Atom* pAtom = static_cast<Atom*>( pEntry->GetUserData() );
        pAtom->compare( pAtom->getCompareAtom() );
    }

    Sync( pPair, 0 );

    return 0;
}

IMPL_LINK(MSViewerWorkWindow, implCollapsingHdl, AtomContainerEntryPair*, pPair )
{
    Sync( pPair, 1 );

    return 0;
}

IMPL_LINK( MSViewerWorkWindow, implMenuHdl, Menu*, pMenu )
{
    if( pMenu )
    {
        USHORT nId = pMenu->GetCurItemId();
        switch( nId )
        {
        case 2: onView(); break;
        case 3: onCompare(); break;
        case 4: Application::Quit(); break;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

MSViewerWorkWindow::~MSViewerWorkWindow()
{
    int nPane;
    for( nPane = 0; nPane < 2; nPane++ )
    {
        delete mpListBox[nPane];
        delete mpEdit[nPane];
    }

    delete mpFileMenu;
    delete mpMenuBar;
}

// -----------------------------------------------------------------------

void MSViewerWorkWindow::Resize()
{
    int nPaneCount = ((mpDocument[0].get() != 0) ? 1 : 0) + ((mpDocument[1].get() != 0) ? 1 : 0);

    Size aOutputSize( GetOutputSizePixel() );
    int nHeight = aOutputSize.Height() >> 1;
    if( nPaneCount )
    {
        int nWidth = aOutputSize.Width();
        if( nPaneCount == 2 )
            nWidth >>= 1;

        int nPosX = 0;

        int nPane;
        for( nPane = 0; nPane < 2; nPane++ )
        {
            mpListBox[nPane]->SetPosSizePixel( nPosX,0, nWidth, nHeight );
            mpEdit[nPane]->SetPosSizePixel( nPosX, nHeight, nWidth, aOutputSize.Height() - nHeight );
            nPosX += nWidth;
        }
    }
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

    SAL_IMPLEMENT_MAIN()
{
    if( argc > 3 )
        return 0;

    uno::Reference< lang::XMultiServiceFactory > xMSF;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx( cppu::defaultBootstrap_InitialComponentContext() );
        if ( !xCtx.is() )
        {
            OSL_FAIL( "Error creating initial component context!" );
            return -1;
        }

        xMSF = uno::Reference< lang::XMultiServiceFactory >(xCtx->getServiceManager(), uno::UNO_QUERY );

        if ( !xMSF.is() )
        {
            OSL_FAIL( "No service manager!" );
            return -1;
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "Exception during creation of initial component context!" );
        return -1;
    }
    comphelper::setProcessServiceFactory( xMSF );

    InitVCL();

    String aConfigURL;
    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aConfigURL ) )
    {
        INetURLObject aURL( aConfigURL );

        aURL.removeSegment();
        aURL.removeFinalSlash();
        aURL.Append( String(   "msview.xml"   ) );

        load_config( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    {
        MSViewerWorkWindow aMainWindow;

        if( argc >= 2 )
        {
            const rtl::OUString aFile1( rtl::OUString::createFromAscii(argv[1]) );
            PPTDocumentPtr pDocument1( new PPTDocument(  aFile1 ) );

            if( argc == 3 )
            {
                const rtl::OUString aFile2( rtl::OUString::createFromAscii(argv[2]) );

                PPTDocumentPtr pDocument2;
                pDocument2.reset( new PPTDocument( aFile2 ) );
                aMainWindow.Compare( pDocument1, pDocument2 );
            }
            else
            {
                aMainWindow.View( pDocument1, 0 );
            }
        }

        aMainWindow.Show();

        Application::Execute();
    }

    DeInitVCL();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
