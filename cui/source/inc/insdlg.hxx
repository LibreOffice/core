/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_INSDLG_HXX
#define _SVX_INSDLG_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
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

    DECL_LINK(          DoubleClickHdl, ListBox* );
    DECL_LINK(          BrowseHdl, PushButton* );
    DECL_LINK(          RadioHdl, RadioButton* );
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

    // get replacement for the iconified embedded object and the mediatype of the replacement
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
};

// class SvInsertPlugInDialog --------------------------------------------

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

