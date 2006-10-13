/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: insdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:22:33 $
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
#ifndef _SVX_INSDLG_HXX
#define _SVX_INSDLG_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#include <svtools/insdlg.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>  // MultiLineEdit
#include <comphelper/embeddedobjectcontainer.hxx>

class INetURLObject;

class InsertObjectDialog_Impl : public ModalDialog
{
protected:
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > m_xObj;
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& m_xStorage;
    comphelper::EmbeddedObjectContainer aCnt;

    InsertObjectDialog_Impl( Window * pParent, const ResId & rResId, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
public:
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()
                        { return m_xObj; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
    virtual BOOL IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    RadioButton aRbNewObject;
    RadioButton aRbObjectFromfile;
    ListBox aLbObjecttype;
    Edit aEdFilepath;
    PushButton aBtnFilepath;
    CheckBox aCbFilelink;
    FixedLine aGbObject;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    String aStrFile;
    String      _aOldStr;
    const SvObjectServerList* m_pServers;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    ::rtl::OUString m_aIconMediaType;

    DECL_LINK(          DoubleClickHdl, ListBox* );
    DECL_LINK(          BrowseHdl, PushButton* );
    DECL_LINK(          RadioHdl, RadioButton* );
    void                SelectDefault();
    ListBox&            GetObjectTypes()
                        { return aLbObjecttype; }
    String              GetFilePath() const { return aEdFilepath.GetText(); }
    BOOL                IsLinked() const    { return aCbFilelink.IsChecked(); }
    BOOL                IsCreateNew() const { return aRbNewObject.IsChecked(); }

public:
    static void         FillObjectServerList( SvObjectServerList* );

                        SvInsertOleDlg( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                            const SvObjectServerList* pServers = NULL );
    virtual short       Execute();

    // get replacement for the iconified embedded object and the mediatype of the replacement
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
};

// class SvInsertPlugInDialog --------------------------------------------

class SvInsertPlugInDialog : public InsertObjectDialog_Impl
{
private:
    Edit aEdFileurl;
    PushButton aBtnFileurl;
    FixedLine aGbFileurl;
    MultiLineEdit aEdPluginsOptions;
    FixedLine aGbPluginsOptions;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    INetURLObject*      m_pURL;
    String              m_aCommands;

    DECL_LINK(          BrowseHdl, PushButton * );
    String              GetPlugInFile() const { return aEdFileurl.GetText(); }
    String              GetPlugInOptions() const { return aEdPluginsOptions.GetText(); }

public:
                        SvInsertPlugInDialog( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );

                        ~SvInsertPlugInDialog();

    virtual short       Execute();
};

// class SvInsertAppletDialog --------------------------------------------

class SvInsertAppletDialog : public InsertObjectDialog_Impl
{
private:
    FixedText aFtClassfile;
    Edit aEdClassfile;
    FixedText aFtClasslocation;
    Edit aEdClasslocation;
    PushButton aBtnClass;
    FixedLine aGbClass;
    MultiLineEdit aEdAppletOptions;
    FixedLine aGbAppletOptions;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    INetURLObject*      m_pURL;
    String              m_aClass;
    String              m_aCommands;
    String              GetClass() const { return aEdClassfile.GetText(); }
    String              GetClassLocation() const { return aEdClasslocation.GetText(); }
    String              GetAppletOptions() const { return aEdAppletOptions.GetText(); }

    void                SetClass( const String &rClass ) { aEdClassfile.SetText(rClass); }
    void                SetClassLocation( const String &rLocation ) { aEdClasslocation.SetText(rLocation); }
    void                SetAppletOptions( const String &rOptions ) { aEdAppletOptions.SetText(rOptions); }
    DECL_LINK(          BrowseHdl, PushButton * );

public:
                        SvInsertAppletDialog( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
                        SvInsertAppletDialog( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
                        ~SvInsertAppletDialog();

    virtual short       Execute();
};

class SfxInsertFloatingFrameDialog : public InsertObjectDialog_Impl
{
private:
    FixedText               aFTName;
    Edit                    aEDName;
    FixedText               aFTURL;
    Edit                    aEDURL;
    PushButton              aBTOpen;

    RadioButton             aRBScrollingOn;
    RadioButton             aRBScrollingOff;
    RadioButton             aRBScrollingAuto;
    FixedLine               aFLScrolling;

    FixedLine               aFLSepLeft;
    RadioButton             aRBFrameBorderOn;
    RadioButton             aRBFrameBorderOff;
    FixedLine               aFLFrameBorder;

    FixedLine               aFLSepRight;
    FixedText               aFTMarginWidth;
    NumericField            aNMMarginWidth;
    CheckBox                aCBMarginWidthDefault;
    FixedText               aFTMarginHeight;
    NumericField            aNMMarginHeight;
    CheckBox                aCBMarginHeightDefault;
    FixedLine               aFLMargin;

    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;

    DECL_STATIC_LINK(       SfxInsertFloatingFrameDialog, OpenHdl, PushButton* );
    DECL_STATIC_LINK(       SfxInsertFloatingFrameDialog, CheckHdl, CheckBox* );

public:
                            SfxInsertFloatingFrameDialog( Window *pParent,
                                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
                            SfxInsertFloatingFrameDialog( Window* pParent,
                                const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
    virtual short           Execute();
};

#endif // _SVX_INSDLG_HXX

