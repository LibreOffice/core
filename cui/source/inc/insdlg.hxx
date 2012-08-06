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
#ifndef _SVX_INSDLG_HXX
#define _SVX_INSDLG_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>

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
    virtual sal_Bool IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    RadioButton aRbNewObject;
    RadioButton aRbObjectFromfile;
    FixedLine aGbObject;
    ListBox aLbObjecttype;
    Edit aEdFilepath;
    PushButton aBtnFilepath;
    CheckBox aCbFilelink;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    String aStrFile;
    String      _aOldStr;
    const SvObjectServerList* m_pServers;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    ::rtl::OUString m_aIconMediaType;

    DECL_LINK(          DoubleClickHdl, void* );
    DECL_LINK(BrowseHdl, void *);
    DECL_LINK(RadioHdl, void *);
    void                SelectDefault();
    ListBox&            GetObjectTypes()
                        { return aLbObjecttype; }
    String              GetFilePath() const { return aEdFilepath.GetText(); }
    sal_Bool                IsLinked() const    { return aCbFilelink.IsChecked(); }
    sal_Bool                IsCreateNew() const { return aRbNewObject.IsChecked(); }

public:
                        SvInsertOleDlg( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                            const SvObjectServerList* pServers = NULL );
    virtual short       Execute();

    /// get replacement for the iconified embedded object and the mediatype of the replacement
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
};

class SvInsertPlugInDialog : public InsertObjectDialog_Impl
{
private:
    FixedLine aGbFileurl;
    Edit aEdFileurl;
    PushButton aBtnFileurl;
    FixedLine aGbPluginsOptions;
    MultiLineEdit aEdPluginsOptions;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    INetURLObject*      m_pURL;
    String              m_aCommands;

    DECL_LINK(BrowseHdl, void *);
    String              GetPlugInFile() const { return aEdFileurl.GetText(); }
    String              GetPlugInOptions() const { return aEdPluginsOptions.GetText(); }

public:
                        SvInsertPlugInDialog( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );

                        ~SvInsertPlugInDialog();

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

    FixedLine               aFLScrolling;
    RadioButton             aRBScrollingOn;
    RadioButton             aRBScrollingOff;
    RadioButton             aRBScrollingAuto;


    FixedLine               aFLSepLeft;
    FixedLine               aFLFrameBorder;
    RadioButton             aRBFrameBorderOn;
    RadioButton             aRBFrameBorderOff;

    FixedLine               aFLSepRight;
    FixedLine               aFLMargin;
    FixedText               aFTMarginWidth;
    NumericField            aNMMarginWidth;
    CheckBox                aCBMarginWidthDefault;
    FixedText               aFTMarginHeight;
    NumericField            aNMMarginHeight;
    CheckBox                aCBMarginHeightDefault;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
