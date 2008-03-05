/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertyeditor.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:14:09 $
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

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROL_HPP_
#include <com/sun/star/inspection/XPropertyControl.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#include <boost/mem_fn.hpp>
#include <map>

//............................................................................
namespace pcr
{
//............................................................................

    class IPropertyLineListener;
    class IPropertyControlObserver;
    class OBrowserPage;
    struct OLineDescriptor;
    class OBrowserListBox;

    //========================================================================
    //= OPropertyEditor
    //========================================================================
    class OPropertyEditor : public Control
    {
    private:
        typedef ::std::map< ::rtl::OUString, sal_uInt16 >   MapStringToPageId;
        struct HiddenPage
        {
            sal_uInt16  nPos;
            TabPage*    pPage;
            HiddenPage() : nPos( 0 ), pPage( NULL ) { }
            HiddenPage( sal_uInt16 _nPos, TabPage* _pPage ) : nPos( _nPos ), pPage( _pPage ) { }
        };

    private:
        TabControl                  m_aTabControl;
        IPropertyLineListener*      m_pListener;
        IPropertyControlObserver*   m_pObserver;
        sal_uInt16                  m_nNextId;
        Link                        m_aPageActivationHandler;
        bool                        m_bHasHelpSection;
        sal_Int32                   m_nMinHelpLines;
        sal_Int32                   m_nMaxHelpLines;

        MapStringToPageId                       m_aPropertyPageIds;
        ::std::map< sal_uInt16, HiddenPage >    m_aHiddenPages;

    protected:
        void                        Resize();
        void                        GetFocus();

    public:
                                    OPropertyEditor (Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL);

                                    ~OPropertyEditor();

        sal_uInt16                  CalcVisibleLines();
        void                        EnableUpdate();
        void                        DisableUpdate();

        void                        SetLineListener( IPropertyLineListener* );
        void                        SetControlObserver( IPropertyControlObserver* );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const ::rtl::OUString& _rHelpText );
        void                        SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );

        void                        SetHelpId( sal_uInt32 nHelpId );
        sal_uInt16                  AppendPage( const String& r, const SmartId& _rHelpId );
        void                        SetPage( sal_uInt16 );
        void                        RemovePage(sal_uInt16 nID);
        sal_uInt16                  GetCurPage();
        void                        ClearAll();

        void                        SetPropertyValue(const ::rtl::OUString& _rEntryName, const ::com::sun::star::uno::Any& _rValue );
        ::com::sun::star::uno::Any  GetPropertyValue(const ::rtl::OUString& rEntryName ) const;
        sal_uInt16                  GetPropertyPos(const ::rtl::OUString& rEntryName ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                    GetPropertyControl( const ::rtl::OUString& rEntryName );
        void                        EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable );
        void                        EnablePropertyControls( const ::rtl::OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );
        sal_Bool                    IsPropertyInputEnabled( const ::rtl::OUString& _rEntryName ) const;

        void                        ShowPropertyPage( sal_uInt16 _nPageId, bool _bShow );

        sal_uInt16                  InsertEntry( const OLineDescriptor&, sal_uInt16 _nPageId, sal_uInt16 nPos = EDITOR_LIST_APPEND );
        void                        RemoveEntry( const ::rtl::OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor& );

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

        // #95343# -------------------------------
        sal_Int32 getMinimumWidth();
        sal_Int32 getMinimumHeight();

        void                        CommitModified();

    protected:
        using Window::SetHelpText;
        using Window::Update;

    private:
        OBrowserPage* getPage( sal_uInt16& _rPageId );
        const OBrowserPage* getPage( sal_uInt16& _rPageId ) const;

        OBrowserPage* getPage( const ::rtl::OUString& _rPropertyName );
        const OBrowserPage* getPage( const ::rtl::OUString& _rPropertyName ) const;

        void Update(const ::std::mem_fun_t<void,OBrowserListBox>& _aUpdateFunction);

        typedef void (OPropertyEditor::*PageOperation)( OBrowserPage&, const void* );
        void    forEachPage( PageOperation _pOperation, const void* _pArgument = NULL );

        void    setPageLineListener( OBrowserPage& _rPage, const void* );
        void    setPageControlObserver( OBrowserPage& _rPage, const void* );
        void    enableHelpSection( OBrowserPage& _rPage, const void* );
        void    setHelpSectionText( OBrowserPage& _rPage, const void* _pPointerToOUString );
        void    setHelpLineLimits( OBrowserPage& _rPage, const void* );

    protected:
        DECL_LINK(OnPageDeactivate, TabControl*);
        DECL_LINK(OnPageActivate, TabControl*);
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_


