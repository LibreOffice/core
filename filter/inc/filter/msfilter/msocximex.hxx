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
#ifndef _MSOCXIMEX_HXX
#define _MSOCXIMEX_HXX

#include <sot/storage.hxx>
#include <tools/debug.hxx>

//!! no such defines in global namespaces - it will break other existing code that uses the same define!!
//#ifndef C2U
//#define C2U(cChar)	rtl::OUString::createFromAscii(cChar)
//#endif
#include "filter/msfilter/msfilterdllapi.h"
#include <vector>
#include <hash_map>

namespace com{namespace sun{namespace star{
        namespace drawing{
                class XDrawPage;
                class XShape;
                class XShapes;
        }
        namespace form{
                class XFormComponent;
        }
        namespace lang{
                class XMultiServiceFactory;
        }
        namespace container{
                class XIndexContainer;
                class XNameContainer;
        }
        namespace beans{
                class XPropertySet;
        }
        namespace text{
                class XText;
        }
        namespace awt{
                struct Size;
                class XControlModel;
        }
        namespace uno{
                class XComponentContext;
        }

}}}

class OCX_Control;
class SfxObjectShell;
class SwPaM;

class TypeName
{
public:
    String msName;
    sal_uInt16 mnType;
    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnStoreId;
    TypeName(sal_Char *pName, sal_uInt32 nStoreId, sal_uInt32 nLen, sal_uInt16 nType,
        sal_Int32 nLeft, sal_Int32 nTop);
};


class MSFILTER_DLLPUBLIC SvxMSConvertOCXControls
{
public:
    SvxMSConvertOCXControls( SfxObjectShell *pDSh,SwPaM *pP );
    virtual ~SvxMSConvertOCXControls();

    //Reads a control from the given storage, constructed shape in pShapeRef
    sal_Bool ReadOCXStream( SotStorageRef& rSrc1,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShapeRef=0,
        BOOL bFloatingCtrl=FALSE );


    //Excel has a nasty kludged mechanism for this, read
    //the comments in the source to follow it
    sal_Bool ReadOCXExcelKludgeStream(SotStorageStreamRef& rSrc1,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape > *pShapeRef,BOOL bFloatingCtrl);


    //Writes the given Uno Control into the given storage

    static sal_Bool WriteOCXStream(SotStorageRef &rSrc1,
        const com::sun::star::uno::Reference<
        com::sun::star::awt::XControlModel > &rControlModel,
        const com::sun::star::awt::Size& rSize,String &rName);

    //Excel has a nasty kludged mechanism for this, read
    //the comments in the source to follow it
    static sal_Bool WriteOCXExcelKludgeStream(SotStorageStreamRef& rContents,
        const com::sun::star::uno::Reference<
        com::sun::star::awt::XControlModel > &rControlModel,
        const com::sun::star::awt::Size &rSize, String &rName);

    //Generate an OCX converter based on the OLE2 name
    static OCX_Control *OCX_Factory(const String &rId);

    //Generate an OCX converter based on the StarOffice UNO id
    static OCX_Control *OCX_Factory( const com::sun::star::uno::Reference<
        com::sun::star::awt::XControlModel > &rControlModel,
        String &rId,String &rName);

    virtual sal_Bool InsertControl(
        const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& /*rFComp*/,
        const com::sun::star::awt::Size& /*rSize*/,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape >* /*pShape*/,
        BOOL /*bFloatingCtrl*/ ) {return sal_False;}

    /*begin: Backwards compatability with office 95 import, modify later*/
    const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & GetServiceFactory();

    sal_uInt16 GetEditNum() { return ++nEdit; }
    sal_uInt16 GetCheckboxNum() { return ++nCheckbox; }
    /*end: Backwards compatability*/
protected:
    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > &
        GetShapes();

    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexContainer > & 	GetFormComps();

    SfxObjectShell *pDocSh;
    SwPaM *pPaM;

    // gecachte Interfaces
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >
        xDrawPage;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >
        xShapes;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > 		xServiceFactory;

    // das einzige Formular
    com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >
        xFormComps;

    sal_uInt16 nEdit;
    sal_uInt16 nCheckbox;

    virtual const com::sun::star::uno::Reference<
        com::sun::star::drawing::XDrawPage > & GetDrawPage();
};

class OCX_FontData
{
public:
    OCX_FontData() : nFontNameLen(0), fBold(0), fItalic(0), fUnderline(0),
        fStrike(0), nFontSize(12), nJustification(1), pFontName(0),
        bHasAlign(FALSE), bHasFont(TRUE) {}
    ~OCX_FontData()  {
        if (pFontName)
        delete [] pFontName;
    }
    sal_Bool Read(SotStorageStream *pS);
    void Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet > &rPropSet);

    sal_Bool Export(SotStorageStreamRef &rContent,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8 pBlockFlags[4];

    sal_uInt32 nFontNameLen;

    sal_uInt8   fBold:1;
    sal_uInt8   fItalic:1;
    sal_uInt8   fUnderline:1;
    sal_uInt8   fStrike:1;
    sal_uInt8   fUnknown1:4;

    sal_uInt8   nUnknown2;
    sal_uInt8   nUnknown3;
    sal_uInt8   nUnknown4;

    sal_uInt32  nFontSize;
    sal_uInt16 nLanguageID;
    sal_uInt8   nJustification;
    sal_uInt16  nFontWeight;

    char *pFontName;
    void SetHasAlign(sal_Bool bIn) {bHasAlign=bIn;}
    void SetHasFont(sal_Bool bIn) {bHasFont=bIn;}
protected:
    static sal_uInt16 nStandardId;
    sal_uInt8 ExportAlign(sal_Int16 nAlign) const;
    sal_Int16 ImportAlign(sal_uInt8 nJustification) const;
private:
    sal_Bool bHasAlign;
    sal_Bool bHasFont;
};

class MSFILTER_DLLPUBLIC OCX_Control
{
public:
    OCX_Control(UniString sN, OCX_Control* parent = NULL ) : nWidth( 0 ), nHeight( 0 ), mnLeft(0), mnTop(0),
        mnStep(0), mnBackColor(0x8000000FL), mnForeColor(0), mnTabPos(0), mbVisible(true), sName(sN), pDocSh(0),
        bSetInDialog(FALSE), mpParent( parent ) {}
    sal_Bool FullRead(SotStorageStream *pS)
    {
        return Read(pS) && ReadFontData(pS);
    };
    virtual sal_Bool Read(SotStorageStream *pS);
    virtual sal_Bool ReadFontData(SotStorageStream *pS);
    virtual sal_Bool Import(const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >  &rServiceFactory,
        com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent > &rFComp,
        com::sun::star::awt::Size &rSz);
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog);
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet>& /*rPropSet*/)
        {return sal_False;}
    //Export exports a control as an OLE style storage stream tree
    virtual sal_Bool Export(SotStorageRef& /*rObj*/,
    const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet>& /*rPropSet*/,
        const com::sun::star::awt::Size& /*rSize*/) {return sal_False;}

    //WriteContents writes the contents of a contents stream, for
    //the Excel export you cannot use Export, only WriteContents instead
    virtual sal_Bool WriteContents(SotStorageStreamRef& /*rObj*/,
    const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet>& /*rPropSet*/,
        const com::sun::star::awt::Size& /*rSize*/) {return sal_False;}
    void SetInDialog(bool bState) { bSetInDialog = bState; }
    bool GetInDialog() { return bSetInDialog; }

    sal_Bool ReadAndImport(SotStorageStream *pS,
        com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >  &rSF,
        com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent > &rFComp,
        com::sun::star::awt::Size &rSz)
    {
        if (Read(pS))
            return Import(rSF,rFComp,rSz);
        return sal_False;
    }
    virtual ~OCX_Control() {}

    static void FillSystemColors();

    sal_uInt32 nWidth;
    sal_uInt32 nHeight;
        sal_Int32 mnLeft;
        sal_Int32 mnTop;
        sal_Int32 mnStep;
    sal_Int32 mnBackColor;
    sal_Int32 mnForeColor;
        sal_uInt16 mnTabPos;
    bool mbVisible;
    UniString sName;
    UniString msToolTip;
    OCX_FontData aFontData;
        SfxObjectShell *pDocSh;
protected:

    sal_uInt32 ImportColor(sal_uInt32 nColorCode) const;
    sal_uInt32 ExportColor(sal_uInt32 nColorCode) const;
    sal_uInt32 SwapColor(sal_uInt32 nColorCode) const;
    sal_Int16 ImportBorder(sal_uInt16 nSpecialEffect,sal_uInt16 nBorderStyle)
        const;
    sal_uInt8 ExportBorder(sal_uInt16 nBorder,sal_uInt8 &rBorderStyle) const;
    bool bSetInDialog;
    sal_Int16 ImportSpecEffect( sal_uInt8 nSpecialEffect ) const;
    sal_uInt8 ExportSpecEffect( sal_Int16 nApiEffect ) const;
    static sal_uInt16 nStandardId;
    static sal_uInt8 __READONLY_DATA aObjInfo[4];
    rtl::OUString msFormType;
    rtl::OUString msDialogType;
        OCX_Control* mpParent;
private:
    static sal_uInt32 pColor[25];
};

class OCX_ModernControl : public OCX_Control
{
public:
    OCX_ModernControl(UniString sN) : OCX_Control(sN),
    fEnabled(1), fLocked(0), fBackStyle(1), fColumnHeads(0), fIntegralHeight(1),
    fMatchRequired(0), fAlignment(1), fDragBehaviour(0), fEnterKeyBehaviour(0),
    fEnterFieldBehaviour(0), fTabKeyBehaviour(0), fWordWrap(1),
    fSelectionMargin(1), fAutoWordSelect(1), fAutoSize(0), fHideSelection(1),
    fAutoTab(0), fMultiLine(1), nMaxLength(0), nBorderStyle(0), nScrollBars(0),
    nStyle(0), nMousePointer(0), nPasswordChar(0), nListWidth(0),
    nBoundColumn(1), nTextColumn(-1), nColumnCount(1), nListRows(8),
    nMatchEntry(2), nListStyle(0), nShowDropButtonWhen(0), nDropButtonStyle(1),
    nMultiState(0), nValueLen(0), nCaptionLen(0), nVertPos(1), nHorzPos(7),
    nSpecialEffect(2), nIcon(0), nPicture(0), nAccelerator(0), nGroupNameLen(0),
    pValue(0), pCaption(0), pGroupName(0), nIconLen(0), pIcon(0),
    nPictureLen(0), pPicture(0) {}

    ~OCX_ModernControl() {
        if (pValue) delete[] pValue;
        if (pCaption) delete[] pCaption;
        if (pGroupName) delete[] pGroupName;
        if (pIcon) delete[] pIcon;
        if (pPicture) delete[] pPicture;
    }
    sal_Bool Read(SotStorageStream *pS);


    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16  nIdentifier;
    sal_uInt16 	nFixedAreaLen;
    sal_uInt8	pBlockFlags[8];

    sal_uInt8	fUnknown1:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown2:4;

    sal_uInt8	fUnknown3:2;
    sal_uInt8	fColumnHeads:1;
    sal_uInt8	fIntegralHeight:1;
    sal_uInt8	fMatchRequired:1;
    sal_uInt8	fAlignment:1;
    sal_uInt8	fUnknown4:2;

    sal_uInt8	fUnknown5:3;
    sal_uInt8	fDragBehaviour:1;
    sal_uInt8	fEnterKeyBehaviour:1;
    sal_uInt8	fEnterFieldBehaviour:1;
    sal_uInt8	fTabKeyBehaviour:1;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown6:2;
    sal_uInt8	fSelectionMargin:1;
    sal_uInt8	fAutoWordSelect:1;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fHideSelection:1;
    sal_uInt8	fAutoTab:1;
    sal_uInt8	fMultiLine:1;

    sal_uInt32 	nMaxLength;
    sal_uInt8	nBorderStyle;
    sal_uInt8	nScrollBars;
    sal_uInt8	nStyle;
    sal_uInt8	nMousePointer;
    sal_uInt8	nUnknown7;
    sal_uInt8	nPasswordChar;
    sal_uInt32	nListWidth;
    sal_uInt16	nBoundColumn;
    sal_Int16	nTextColumn;
    sal_uInt16	nColumnCount;
    sal_uInt16	nListRows;
    sal_uInt16	nUnknown8;
    sal_uInt8	nMatchEntry;
    sal_uInt8	nListStyle;
    sal_uInt8	nShowDropButtonWhen;
    sal_uInt8	nDropButtonStyle;
    sal_uInt8	nMultiState;
    sal_uInt32	nValueLen;
    sal_uInt32	nCaptionLen;

    sal_uInt16 nVertPos;
    sal_uInt16 nHorzPos;

    sal_uInt32 nBorderColor;
    sal_uInt8 nSpecialEffect;
    sal_uInt16 nIcon;
    sal_uInt16 nPicture;
    sal_uInt8 nAccelerator;
    sal_uInt8 nUnknown9;
    sal_uInt32 nGroupNameLen;
    sal_uInt32 nUnknown10;

    char *pValue;
    char *pCaption;
    char *pGroupName;

    sal_uInt8 pIconHeader[20];
    sal_uInt32 nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32 nPictureLen;
    sal_uInt8 *pPicture;

};

class OCX_TabStrip : public OCX_Control
{
public:
    OCX_TabStrip() : OCX_Control( rtl::OUString::createFromAscii("TabStrip")) {}
        virtual sal_Bool ReadFontData(SotStorageStream *pS);
        virtual sal_Bool Read(SotStorageStream *pS);

    sal_uInt16	nIdentifier;
    sal_uInt16	nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];
    sal_uInt16	nNumTabs;
};

class OCX_Image : public OCX_Control
{
public:
    OCX_Image() : OCX_Control(rtl::OUString::createFromAscii("Image")), fEnabled(1), fBackStyle(0), bPictureTiling(false), bAutoSize(false) {
                msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseImageControl");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlImageControlModel");
         }

    ~OCX_Image() { }
    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16	nIdentifier;
    sal_uInt16	nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];

    sal_uInt32	nBorderColor;
    sal_uInt8	nBorderStyle;
    sal_uInt8 	nMousePointer;
        sal_uInt8       nPictureSizeMode;


           sal_uInt8	fUnknown1:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fUnknown2:2;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown3:3;

        sal_uInt8       nPictureAlignment;
        bool            bPictureTiling;
    sal_uInt8	nSpecialEffect;

        bool bAutoSize;
        ::rtl::OUString sImageUrl;
        sal_Bool Read(SotStorageStream *pS);

    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet>& rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
        // No Font record
    virtual sal_Bool ReadFontData(SvStorageStream* /*pS*/) { return sal_True; }

    static OCX_Control *Create() { return new OCX_Image;}
};
struct ContainerRecord
{
    ContainerRecord():nTop(0), nLeft(0), nSubStorageId(0), nSubStreamLen(0), nTabPos(0), nTypeIdent(0), bVisible( true ) {}

    ::rtl::OUString cName;
    ::rtl::OUString controlTip;

    sal_uInt32 nTop;
    sal_uInt32 nLeft;
    sal_uInt32 nSubStorageId;
    sal_uInt32 nSubStreamLen;
    sal_uInt16  nTabPos;
    sal_uInt16 nTypeIdent;
    bool bVisible;
};

typedef std::vector<OCX_Control*>::iterator CtrlIterator;
typedef std::vector<OCX_Control*>::const_iterator CtrlIteratorConst;
typedef std::vector<OCX_Control*>  CtrlList;



class RBGroup
{
    public:
    RBGroup():mRBGroupPos(0){}
    RBGroup(sal_uInt16& groupPos ):mRBGroupPos(groupPos){}
    sal_Int16 tabPos() const { return mRBGroupPos; }
    std::vector<OCX_Control*>::size_type numControls()
    { return mpControls.size(); }
    std::vector<OCX_Control*>& controls() { return mpControls; }

    void add(OCX_Control* pRB);
    private:
    sal_uInt16 mRBGroupPos;
    std::vector<OCX_Control*> mpControls;
};

typedef ::std::hash_map < ::rtl::OUString, RBGroup*, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > RBGroupHash;
typedef std::vector<RBGroup*>::iterator GroupIterator;

class OCX_OptionButton;

class RBGroupManager
{
public:
    RBGroupManager( String& defaultName );
    ~RBGroupManager();

    CtrlList insertGroupsIntoControlList( const CtrlList& sourceList );
    void addRadioButton( OCX_OptionButton* pRButton );
private:

    void addSeperator( std::vector< OCX_Control* >& dest );
    void copyList( std::vector< OCX_Control* >& src,
                  std::vector< OCX_Control* >& dest,
                  bool addGroupSeperator );

    RBGroupHash rbGroups;
    String mSDefaultName;
    std::vector< RBGroup* > groupList;
    sal_uInt16 numRadioButtons;
};




class OCX_ContainerControl : public OCX_Control
{
public:
    virtual ~OCX_ContainerControl();
        // sub class will process the control specific information
        // e.g frame or userform ( maybe tab, mulipage in the future )
        // Base (this) class will process the container specific information
        // e.g. the controls contained by this container
        // will
        // a) create the controls
        // b) read the controls
        // c) store these controls in a list for post processing
        //     e.g. import
        //
    virtual sal_Bool Read(SvStorageStream *pS);
        // No Font record
    virtual sal_Bool ReadFontData(SvStorageStream* /*pS*/) { return sal_True; }

    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

        SotStorageStreamRef getContainerStream() { return mContainerStream; }

        virtual void ProcessControl( OCX_Control* pControl, SvStorageStream* pS, ContainerRecord& rec );
        bool createFromContainerRecord( const ContainerRecord& record,
            OCX_Control*& );
protected:
        // This class not meant to be instantiated
        // needs to be subclassed
    OCX_ContainerControl( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer >  &rParent,
            OCX_Control* pParent = NULL );
        rtl::OUString createSubStreamName( const sal_uInt32& subStorageID );

        RBGroupManager rbGroupMgr;
        com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer > mxParent;
    std::vector<OCX_Control*> mpControls;
        SotStorageRef mContainerStorage;
        SotStorageStreamRef mContainerStream;
        SotStorageStreamRef mContainedControlsStream;
    sal_uInt32 nNoRecords;
    sal_uInt32 nTotalLen;
        sal_uInt32 containerType;

private:
        OCX_ContainerControl(); // not implemented
        OCX_ContainerControl(const OCX_ContainerControl&); // not implemented
};


class OCX_MultiPage : public OCX_ContainerControl
{
public:
    OCX_MultiPage( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer >  &rDialog, OCX_Control* pParent = NULL);
    virtual ~OCX_MultiPage()
    {
        delete[] pCaption;
        delete[] pIcon;
        delete[] pPicture;
    }
    virtual sal_Bool Read(SvStorageStream *pS);

    using OCX_ContainerControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog);
        virtual void ProcessControl( OCX_Control* pControl, SvStorageStream* pS, ContainerRecord& rec );
    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];

    sal_uInt32  fUnknown1;

    sal_uInt8	fUnknown2:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown3:4;

    sal_uInt8	fUnknown4:8;

    sal_uInt8	fUnknown5:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown6:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown7:3;

    sal_uInt32	nCaptionLen;
    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;
    sal_uInt8 	nMousePointer;
    sal_uInt32	nBorderColor;
    sal_uInt32  fUnknown8;
    sal_uInt32  fUnknown9;
    sal_uInt8   nKeepScrollBarsVisible;
    sal_uInt8   nCycle;
    sal_uInt16	nBorderStyle;
    sal_uInt16	nSpecialEffect;
    sal_uInt16	nPicture;
    sal_uInt8   nPictureAlignment;
    sal_uInt8   nPictureSizeMode;
    bool        bPictureTiling;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;

    char *pCaption;

    sal_uInt32 	nScrollWidth;
    sal_uInt32 	nScrollHeight;


    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;
private:
        sal_Int32 mnCurrentPageStep;
};



class OCX_Page : public OCX_ContainerControl
{
public:
    OCX_Page( SotStorageRef& parentStorage,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer >  &rDialog, OCX_Control* parent = NULL);
    virtual ~OCX_Page()
    {
        delete[] pCaption;
        delete[] pIcon;
        delete[] pPicture;
    }
    virtual sal_Bool Read(SvStorageStream *pS);

    using OCX_ContainerControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog);
/*	virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
*/
    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];

    sal_uInt32  fUnknown1;

    sal_uInt8	fUnknown2:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown3:4;

    sal_uInt8	fUnknown4:8;

    sal_uInt8	fUnknown5:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown6:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown7:3;

    sal_uInt32	nCaptionLen;
    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;
    sal_uInt8 	nMousePointer;
    sal_uInt32	nBorderColor;
    sal_uInt32  fUnknown8;
    sal_uInt32  fUnknown9;
    sal_uInt8   nKeepScrollBarsVisible;
    sal_uInt8   nCycle;
    sal_uInt16	nBorderStyle;
    sal_uInt16	nSpecialEffect;
    sal_uInt16	nPicture;
    sal_uInt8   nPictureAlignment;
    sal_uInt8   nPictureSizeMode;
    bool        bPictureTiling;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;

    char *pCaption;

    sal_uInt32 	nScrollWidth;
    sal_uInt32 	nScrollHeight;


    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;
private:
};


class OCX_Frame : public OCX_ContainerControl
{
public:
    OCX_Frame( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer >  &rDialog, OCX_Control* pParent = NULL);
    virtual ~OCX_Frame()
    {
        delete[] pCaption;
        delete[] pIcon;
        delete[] pPicture;
    }
    virtual sal_Bool Read(SvStorageStream *pS);

    using OCX_ContainerControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];

    sal_uInt32  fUnknown1;

    sal_uInt8	fUnknown2:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown3:4;

    sal_uInt8	fUnknown4:8;

    sal_uInt8	fUnknown5:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown6:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown7:3;

    sal_uInt32	nCaptionLen;
    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;
    sal_uInt8 	nMousePointer;
    sal_uInt32	nBorderColor;
    sal_uInt32  fUnknown8;
    sal_uInt32  fUnknown9;
    sal_uInt8   nKeepScrollBarsVisible;
    sal_uInt8   nCycle;
    sal_uInt16	nBorderStyle;
    sal_uInt16	nSpecialEffect;
    sal_uInt16	nPicture;
    sal_uInt8   nPictureAlignment;
    sal_uInt8   nPictureSizeMode;
    bool        bPictureTiling;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;

    char *pCaption;

    sal_uInt32 	nScrollWidth;
    sal_uInt32 	nScrollHeight;
    sal_uInt32 	nScrollLeft;
    sal_uInt32 	nScrollTop;


    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;
private:
};

class OCX_UserForm : public OCX_ContainerControl
{
public:
    OCX_UserForm( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const com::sun::star::uno::Reference<
                com::sun::star::container::XNameContainer >  &rDialog,
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rMsf);
    ~OCX_UserForm()
    {
        delete[] pCaption;
        delete[] pIcon;
        delete[] pPicture;
    }

    virtual sal_Bool Read(SvStorageStream *pS);

    using OCX_ContainerControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import( com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog);

    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];

    sal_uInt32  nChildrenA;

    sal_uInt8	fUnknown1:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown2:4;

    sal_uInt8	fUnknown3:8;

    sal_uInt8	fUnknown4:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown5:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown6:3;

    sal_uInt32	nCaptionLen;
    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;
    sal_uInt8 	nMousePointer;
    sal_uInt32	nBorderColor;
    sal_uInt32  nDrawBuffer;
    sal_uInt32  nChildrenB;
    sal_uInt8   nKeepScrollBarsVisible;
    sal_uInt8   nCycle;
    sal_uInt16	nBorderStyle;
    sal_uInt8	nSpecialEffect;
    sal_uInt16	nPicture;
    sal_uInt8   nPictureAlignment;
    sal_uInt8   nPictureSizeMode;
    bool        bPictureTiling;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;
    sal_uInt16	fUnknown7;

    char *pCaption;

    sal_uInt32 	nScrollWidth;
    sal_uInt32 	nScrollHeight;
    sal_uInt32 	nScrollLeft;
    sal_uInt32 	nScrollTop;

    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;
private:
        com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext> mxCtx;
};



class OCX_CheckBox : public OCX_ModernControl
{
public:
    OCX_CheckBox() : OCX_ModernControl(rtl::OUString::createFromAscii("CheckBox")){
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.CheckBox");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlCheckBoxModel");
        mnBackColor = 0x80000005L;
        mnForeColor = 0x80000008L;
        aFontData.SetHasAlign(TRUE);
    }

    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    static OCX_Control *Create() { return new OCX_CheckBox;}
};

class OCX_OptionButton : public OCX_ModernControl
{
public:
    OCX_OptionButton() : OCX_ModernControl(rtl::OUString::createFromAscii("OptionButton"))
    {
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.RadioButton");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlRadioButtonModel");
        mnBackColor = 0x80000005L;
        mnForeColor = 0x80000008L;
        aFontData.SetHasAlign(TRUE);
    }

    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);


    static OCX_Control *Create() { return new OCX_OptionButton;}
};

class OCX_TextBox : public OCX_ModernControl
{
public:
    OCX_TextBox() : OCX_ModernControl(rtl::OUString::createFromAscii("TextBox")) {
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.TextField");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlEditModel");
        mnBackColor = 0x80000005L;
        mnForeColor = 0x80000008L;
        nBorderColor = 0x80000006L;
        aFontData.SetHasAlign(TRUE);
    }

    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    static OCX_Control *Create() { return new OCX_TextBox;}
};

class OCX_FieldControl: public OCX_ModernControl
{
public:
    OCX_FieldControl() : OCX_ModernControl(rtl::OUString::createFromAscii("TextBox")) {
        mnBackColor = 0x80000005L;
        mnForeColor = 0x80000008L;
        nBorderColor = 0x80000006L;
    }
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    static OCX_Control *Create() { return new OCX_FieldControl;}
};


class OCX_ToggleButton : public OCX_ModernControl
{
public:
    OCX_ToggleButton() : OCX_ModernControl(rtl::OUString::createFromAscii("ToggleButton")) {
                msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.CommandButton");
                msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlButtonModel");

        mnBackColor = 0x8000000F;
        mnForeColor = 0x80000012;
        aFontData.SetHasAlign(TRUE);
        aFontData.nJustification = 3;   // centered by default
    }
    static OCX_Control *Create() { return new OCX_ToggleButton;}

    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SvStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SvStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
};

class OCX_ComboBox : public OCX_ModernControl
{
public:
    OCX_ComboBox() : OCX_ModernControl(rtl::OUString::createFromAscii("ComboBox")){
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.ComboBox");
            msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlComboBoxModel");
        mnBackColor = 0x80000005;
        mnForeColor = 0x80000008;
        nBorderColor = 0x80000006;
        aFontData.SetHasAlign(TRUE);
    }
    static OCX_Control *Create() { return new OCX_ComboBox;}
    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
};

class OCX_ListBox : public OCX_ModernControl
{
public:
    OCX_ListBox() : OCX_ModernControl(rtl::OUString::createFromAscii("ListBox")){
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.ListBox");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlListBoxModel");
        mnBackColor = 0x80000005;
        mnForeColor = 0x80000008;
        nBorderColor = 0x80000006;
        aFontData.SetHasAlign(TRUE);
    }
    static OCX_Control *Create() { return new OCX_ListBox;}
    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);

};

class OCX_CommandButton : public OCX_Control
{
public:
    OCX_CommandButton() : OCX_Control(String::CreateFromAscii("CommandButton")),
    fEnabled(1), fLocked(0), fBackStyle(1), fWordWrap(0), fAutoSize(0),
        nCaptionLen(0), nVertPos(1), nHorzPos(7), nMousePointer(0), nPicture(0),
        nAccelerator(0), nIcon(0), pCaption(0), nIconLen(0), pIcon(0), nPictureLen(0),
        pPicture(0), mbTakeFocus( true )
    {
            msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.CommandButton");
            msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlButtonModel");
            mnForeColor = 0x80000012L;
                mnBackColor = 0x8000000FL;
    }

    ~OCX_CommandButton() {
        if (pCaption) delete[] pCaption;
        if (pIcon) delete[] pIcon;
        if (pPicture) delete[] pPicture;
    }
    sal_Bool Read(SotStorageStream *pS);

    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16	nIdentifier;
    sal_uInt16	nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];


    sal_uInt8	fUnknown1:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown2:4;

    sal_uInt8	fUnknown3:8;

    sal_uInt8	fUnknown4:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown5:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown6:3;

    sal_uInt32	nCaptionLen;

    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;

    sal_uInt16 	nMousePointer;
    sal_uInt16	nPicture;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;

    char *pCaption;

    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;

    bool        mbTakeFocus;

    static OCX_Control *Create() { return new OCX_CommandButton;}
    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);

};

class OCX_ImageButton: public OCX_CommandButton
{
public:
    OCX_ImageButton() {
        aFontData.SetHasAlign(FALSE);
        aFontData.SetHasFont(FALSE);
    }
    static OCX_Control *Create() { return new OCX_ImageButton;}
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);

};

class OCX_GroupBox: public OCX_Control
{
public:
    OCX_GroupBox() : OCX_Control(String::CreateFromAscii("GroupBox")) {}
    static OCX_Control *Create() { return new OCX_GroupBox;}
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);

};

class OCX_Label : public OCX_Control
{
public:
    OCX_Label(OCX_Control* pParent = NULL ) : OCX_Control(rtl::OUString::createFromAscii("Label"), pParent ), fEnabled(1),
        fLocked(0),fBackStyle(1),fWordWrap(1),
    fAutoSize(0),nCaptionLen(0),nVertPos(1),nHorzPos(7),nMousePointer(0),
    nBorderColor(0x80000006),nBorderStyle(0),nSpecialEffect(0),
    nPicture(0),nAccelerator(0),nIcon(0),pCaption(0),nIconLen(0),pIcon(0),
    nPictureLen(0),pPicture(0)
    {
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.FixedText");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedTextModel");
                mnForeColor = 0x80000008;
                mnBackColor = 0x80000005;
        aFontData.SetHasAlign(TRUE);
    }

    ~OCX_Label() {
        if (pCaption) delete[] pCaption;
        if (pIcon) delete[] pIcon;
        if (pPicture) delete[] pPicture;
    }
    sal_Bool Read(SotStorageStream *pS);

    /*sal_uInt8 for sal_uInt8 Word Struct*/
    sal_uInt16 nIdentifier;
    sal_uInt16 nFixedAreaLen;
    sal_uInt8	pBlockFlags[4];


    sal_uInt8	fUnknown1:1;
    sal_uInt8	fEnabled:1;
    sal_uInt8	fLocked:1;
    sal_uInt8	fBackStyle:1;
    sal_uInt8	fUnknown2:4;

    sal_uInt8	fUnknown3:8;

    sal_uInt8	fUnknown4:7;
    sal_uInt8	fWordWrap:1;

    sal_uInt8	fUnknown5:4;
    sal_uInt8	fAutoSize:1;
    sal_uInt8	fUnknown6:3;

    sal_uInt32	nCaptionLen;
    sal_uInt16  nVertPos;
    sal_uInt16  nHorzPos;
    sal_uInt8 	nMousePointer;
    sal_uInt32	nBorderColor;
    sal_uInt16	nBorderStyle;
    sal_uInt16	nSpecialEffect;
    sal_uInt16	nPicture;
    sal_uInt16	nAccelerator;
    sal_uInt16	nIcon;

    char *pCaption;

    sal_uInt8 pIconHeader[20];
    sal_uInt32  nIconLen;
    sal_uInt8 *pIcon;

    sal_uInt8 pPictureHeader[20];
    sal_uInt32  nPictureLen;
    sal_uInt8 *pPicture;

    static OCX_Control *Create() { return new OCX_Label;}

    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);

};


class OCX_ProgressBar : public OCX_Control
{
public:
    explicit            OCX_ProgressBar();

    static OCX_Control* Create();

    virtual sal_Bool    Read( SvStorageStream *pS );
    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySet> &rPropSet);
        // No Font record
    virtual sal_Bool ReadFontData(SvStorageStream* /*pS*/) { return sal_True; }
private:
    sal_Int32   nMin;
    sal_Int32   nMax;
    bool        bFixedSingle;
    bool        bEnabled;
    bool        b3d;
};

class OCX_SpinButton : public OCX_Control
{
public:
    explicit            OCX_SpinButton();

    static OCX_Control* Create();

    virtual sal_Bool    Read( SvStorageStream *pS );

    virtual sal_Bool    ReadFontData( SvStorageStream *pS );

    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
    //Export exports a control as an OLE style storage stream tree
    virtual sal_Bool    Export( SvStorageRef &rObj,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet> &rPropSet,
                                const com::sun::star::awt::Size& rSize );

    //WriteContents writes the contents of a contents stream, for
    //the Excel export you cannot use Export, only WriteContents instead
    virtual sal_Bool    WriteContents( SvStorageStreamRef &rObj,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet> &rPropSet,
                                const com::sun::star::awt::Size& rSize );

protected:
    void                UpdateInt32Property(
                            sal_Int32& rnCoreValue, sal_Int32 nNewValue,
                            sal_Int32 nBlockFlag );

    void                GetInt32Property(
                            sal_Int32& rnCoreValue,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet >& rxPropSet,
                            const rtl::OUString& rPropName,
                            sal_Int32 nBlockFlag );

    void                UpdateBoolProperty(
                            bool& rbCoreValue, bool bNewValue,
                            sal_Int32 nBlockFlag );

    void                GetBoolProperty(
                            bool& rbCoreValue,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet >& rxPropSet,
                            const rtl::OUString& rPropName,
                            sal_Int32 nBlockFlag );

    /** Writes the data from own members to stream. */
    sal_Bool            WriteData( SvStream& rStrm ) const;

    sal_Int32           mnBlockFlags;
    sal_Int32           mnValue;
    sal_Int32           mnMin;
    sal_Int32           mnMax;
    sal_Int32           mnSmallStep;
    sal_Int32           mnPageStep;
    sal_Int32           mnOrient;
    sal_Int32           mnDelay;
    bool                mbEnabled;
    bool                mbLocked;
    bool                mbPropThumb;
};

class OCX_ScrollBar : public OCX_SpinButton
{
public:
    explicit            OCX_ScrollBar();

    static OCX_Control* Create();

    using OCX_Control::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);

    //Export exports a control as an OLE style storage stream tree
    virtual sal_Bool    Export( SvStorageRef &rObj,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet> &rPropSet,
                                const com::sun::star::awt::Size& rSize );

    //WriteContents writes the contents of a contents stream, for
    //the Excel export you cannot use Export, only WriteContents instead
    virtual sal_Bool    WriteContents( SvStorageStreamRef &rObj,
                            const com::sun::star::uno::Reference<
                                com::sun::star::beans::XPropertySet> &rPropSet,
                                const com::sun::star::awt::Size& rSize );
};


class HTML_TextBox : public OCX_ModernControl
{
public:
    HTML_TextBox() : OCX_ModernControl(rtl::OUString::createFromAscii("TextBox")) {
        msFormType = rtl::OUString::createFromAscii("com.sun.star.form.component.TextField");
        msDialogType = rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlEditModel");
        mnBackColor = 0x80000005L;
        mnForeColor = 0x80000008L;
        nBorderColor = 0x80000006L;
        aFontData.SetHasAlign(TRUE);
    }

    using OCX_ModernControl::Import; // to not hide the other two import methods
    virtual sal_Bool Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet);
  /*
    sal_Bool Export(SotStorageRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
    sal_Bool WriteContents(SotStorageStreamRef &rObj,
        const com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet,
        const com::sun::star::awt::Size& rSize);
  */
    static OCX_Control *Create() { return new HTML_TextBox;}

        virtual sal_Bool Read(SotStorageStream *pS);
        virtual sal_Bool ReadFontData(SotStorageStream *pS);
};



#endif
