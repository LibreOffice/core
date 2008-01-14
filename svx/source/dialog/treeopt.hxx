/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeopt.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:21:25 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_FIXBRD_HXX //autogen
#include <vcl/fixbrd.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#include <vector>

// static ----------------------------------------------------------------

BOOL            EnableSSO();
CreateTabPage   GetSSOCreator( void );

// class OfaOptionsTreeListBox -------------------------------------------

//!#define NUMBER_OF_OPTION_PAGES  12
class SfxModule;
class SfxShell;
class SfxItemSet;
class XColorTable;
class OfaOptionsTreeListBox : public SvTreeListBox
{
    using  SvListView::Collapse;

private:
    BOOL            bInCollapse;

public:
    OfaOptionsTreeListBox(Window* pParent, const ResId& rResId) :
        SvTreeListBox( pParent, rResId ), bInCollapse(FALSE) {}

    virtual BOOL    Collapse( SvLBoxEntry* pParent );
    BOOL            IsInCollapse()const {return bInCollapse;}
};

// struct OrderedEntry ---------------------------------------------------

struct OrderedEntry
{
    sal_Int32       m_nIndex;
    rtl::OUString   m_sId;

    OrderedEntry( sal_Int32 nIndex, const rtl::OUString& rId ) :
        m_nIndex( nIndex ), m_sId( rId ) {}
};

typedef std::vector< OrderedEntry* > VectorOfOrderedEntries;

// struct Module ---------------------------------------------------------

struct Module
{
    rtl::OUString           m_sName;
    bool                    m_bActive;
    VectorOfOrderedEntries  m_aNodeList;

    Module( const rtl::OUString& rName ) : m_sName( rName ), m_bActive( false ) {}
};

// struct OptionsLeaf ----------------------------------------------------

struct OptionsLeaf
{
    rtl::OUString   m_sId;
    rtl::OUString   m_sLabel;
    rtl::OUString   m_sPageURL;
    rtl::OUString   m_sEventHdl;
    rtl::OUString   m_sGroupId;
    sal_Int32       m_nGroupIndex;

    OptionsLeaf(    const rtl::OUString& rId,
                    const rtl::OUString& rLabel,
                    const rtl::OUString& rPageURL,
                    const rtl::OUString& rEventHdl,
                    const rtl::OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_sEventHdl( rEventHdl ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}
};

typedef ::std::vector< OptionsLeaf* > VectorOfLeaves;
typedef ::std::vector< VectorOfLeaves > VectorOfGroupedLeaves;

// struct OptionsNode ----------------------------------------------------

struct OptionsNode
{
    rtl::OUString           m_sId;
    rtl::OUString           m_sLabel;
    rtl::OUString           m_sPageURL;
    bool                    m_bAllModules;
    rtl::OUString           m_sGroupId;
    sal_Int32               m_nGroupIndex;
    VectorOfLeaves          m_aLeaves;
    VectorOfGroupedLeaves   m_aGroupedLeaves;

    OptionsNode(    const rtl::OUString& rId,
                    const rtl::OUString& rLabel,
                    const rtl::OUString& rPageURL,
                    bool bAllModules,
                    const rtl::OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_bAllModules( bAllModules ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}

    ~OptionsNode()
    {
        for ( sal_uInt32 i = 0; i < m_aLeaves.size(); ++i )
            delete m_aLeaves[i];
        m_aLeaves.clear();
        m_aGroupedLeaves.clear();
    }
};

typedef ::std::vector< OptionsNode* > VectorOfNodes;

struct LastPageSaver
{
    USHORT          m_nLastPageId;
    rtl::OUString   m_sLastPageURL_Tools;
    rtl::OUString   m_sLastPageURL_ExtMgr;

    LastPageSaver() : m_nLastPageId( USHRT_MAX ) {}
};

// class OfaTreeOptionsDialog --------------------------------------------

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }
namespace com { namespace sun { namespace star { namespace container { class XNameAccess; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowProvider; } } } }

struct OptionsPageInfo;
struct Module;
class ExtensionsTabPage;
typedef std::vector< ExtensionsTabPage* > VectorOfPages;

class OfaTreeOptionsDialog : public SfxModalDialog
{
private:
    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    PushButton      aBackPB;

    FixedBorder     aHiddenGB;
    FixedText       aPageTitleFT;
    FixedLine       aLine1FL;
    FixedText       aHelpFT;
    FixedImage      aHelpImg;

    ImageList       aPageImages;
    ImageList       aPageImagesHC;

    ResStringArray  aHelpTextsArr;

    OfaOptionsTreeListBox   aTreeLB;

    String          sTitle;
    String          sNotLoadedError;

    SvLBoxEntry*    pCurrentPageEntry;

    // for the ColorTabPage
    SfxItemSet*     pColorPageItemSet;
    XColorTable*    pColorTab;
    USHORT          nChangeType;
    USHORT          nUnknownType;
    USHORT          nUnknownPos;
    BOOL            bIsAreaTP;

    BOOL            bForgetSelection;
    BOOL            bExternBrowserActive;
    BOOL            bImageResized;
    bool            bInSelectHdl_Impl;
    bool            bIsFromExtensionManager;

    // check "for the current document only" and set focus to "Western" languages box
    bool            bIsForSetDocumentLanguage;

    Timer           aSelectTimer;

    com::sun::star::uno::Reference < com::sun::star::awt::XContainerWindowProvider >
                    m_xContainerWinProvider;

    static LastPageSaver*   pLastPageSaver;

    SfxItemSet*     CreateItemSet( USHORT nId );
    void            ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    void            InitTreeAndHandler();
    void            Initialize( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame );
    void            ResizeTreeLB( void );   // resizes dialog so that treelistbox has no horizontal scroll bar

    void            LoadExtensionOptions( const rtl::OUString& rExtensionId );
    rtl::OUString   GetModuleIdentifier( const com::sun::star::uno::Reference<
                                            com::sun::star::lang::XMultiServiceFactory >& xMFac,
                                         const com::sun::star::uno::Reference<
                                            com::sun::star::frame::XFrame >& xFrame );
    Module*         LoadModule( const rtl::OUString& rModuleIdentifier,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess >& xRoot );
    void            LoadNodes( const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess >& xRoot,
                               Module* pModule,
                               const rtl::OUString& rExtensionId,
                               VectorOfNodes& rOutNodeList );
    void            InsertNodes( const VectorOfNodes& rNodeList );

protected:
    DECL_LINK(ExpandedHdl_Impl, SvTreeListBox* );
    DECL_LINK(ShowPageHdl_Impl, SvTreeListBox* );
    DECL_LINK(BackHdl_Impl, PushButton* );
    DECL_LINK( OKHdl_Impl, Button * );
    DECL_LINK( HintHdl_Impl, Timer * );
    DECL_LINK( SelectHdl_Impl, Timer * );

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual short   Execute();

public:
    OfaTreeOptionsDialog( Window* pParent,
        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame,
        bool bActivateLastSelection = true );
    OfaTreeOptionsDialog( Window* pParent, const rtl::OUString& rExtensionId );
    ~OfaTreeOptionsDialog();

    OptionsPageInfo*    AddTabPage( USHORT nId, const String& rPageName, USHORT nGroup );
    USHORT              AddGroup(   const String& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, USHORT nDialogId );

    void                ActivateLastSelection();
    void                ActivatePage( USHORT nResId );
    void                ActivatePage( const String& rPageURL );
    void                ApplyItemSets();

    USHORT              GetColorChanged() const { return nChangeType; }
    XColorTable*        GetColorTable() { return pColorTab; }

    // helper functions to call the language settings TabPage from the SpellDialog
    static void         ApplyLanguageOptions(const SfxItemSet& rSet);
};

// class OfaPageResource -------------------------------------------------

class OfaPageResource : public Resource
{
    ResStringArray      aGeneralDlgAry;
    ResStringArray      aInetDlgAry;
    ResStringArray      aLangDlgAry;
    ResStringArray      aTextDlgAry;
    ResStringArray      aHTMLDlgAry;
    ResStringArray      aCalcDlgAry;
    ResStringArray      aStarMathDlgAry;
    ResStringArray      aImpressDlgAry;
    ResStringArray      aDrawDlgAry;
    ResStringArray      aChartDlgAry;
    ResStringArray      aFilterDlgAry;
    ResStringArray      aDatasourcesDlgAry;

public:
    OfaPageResource();

    ResStringArray& GetGeneralArray()       {return aGeneralDlgAry;}
    ResStringArray& GetInetArray()          {return aInetDlgAry;}
    ResStringArray& GetLangArray()          {return aLangDlgAry;}
    ResStringArray& GetTextArray()          {return aTextDlgAry;}
    ResStringArray& GetHTMLArray()          {return aHTMLDlgAry;}
    ResStringArray& GetCalcArray()          {return aCalcDlgAry;}
    ResStringArray& GetStarMathArray()      {return aStarMathDlgAry;}
    ResStringArray& GetImpressArray()       {return aImpressDlgAry;}
    ResStringArray& GetDrawArray()          {return aDrawDlgAry;}
    ResStringArray& GetChartArray()         {return aChartDlgAry;}
    ResStringArray& GetFilterArray()        {return aFilterDlgAry;}
    ResStringArray& GetDatasourcesArray()   {return aDatasourcesDlgAry;}
};

// class ExtensionsTabPage -----------------------------------------------

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowEventHandler; } } } }

class ExtensionsTabPage : public TabPage
{
private:
    rtl::OUString       m_sPageURL;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >
                        m_xPage;
    rtl::OUString       m_sEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowEventHandler >
                        m_xEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowProvider >
                        m_xWinProvider;
    bool                m_bIsWindowHidden;

    void                CreateDialogWithHandler();
    sal_Bool            DispatchAction( const rtl::OUString& rAction );

public:
    ExtensionsTabPage(
        Window* pParent, const ResId&,
        const rtl::OUString& rPageURL, const rtl::OUString& rEvtHdl,
        const com::sun::star::uno::Reference<
            com::sun::star::awt::XContainerWindowProvider >& rProvider );

    ExtensionsTabPage(
        Window* pParent, WinBits nStyle,
        const rtl::OUString& rPageURL, const rtl::OUString& rEvtHdl,
        const com::sun::star::uno::Reference<
            com::sun::star::awt::XContainerWindowProvider >& rProvider );

    virtual ~ExtensionsTabPage();

    virtual void    ActivatePage();
    virtual void    DeactivatePage();

    void            ResetPage();
    void            SavePage();
    void            HideWindow();
};

