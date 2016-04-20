/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "fielduno.hxx"
#include "textuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "cellsuno.hxx"
#include "servuno.hxx"
#include "unonames.hxx"
#include "editutil.hxx"

#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>

#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace {

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

const SfxItemPropertySet* getDateTimePropertySet()
{
    static const SfxItemPropertyMapEntry aMapContent[] =
    {
        { OUString(SC_UNONAME_DATETIME), 0, cppu::UnoType<util::DateTime>::get(), 0, 0 },
        { OUString(SC_UNONAME_ISFIXED),  0, cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(SC_UNONAME_ISDATE),   0, cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(SC_UNONAME_NUMFMT),   0, cppu::UnoType<sal_Int32>::get(),      0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aMap(aMapContent);
    return &aMap;
}

const SfxItemPropertySet* getEmptyPropertySet()
{
    static const SfxItemPropertyMapEntry aMapContent[] =
    {
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aMap(aMapContent);
    return &aMap;
}

const SfxItemPropertySet* lcl_GetURLPropertySet()
{
    static const SfxItemPropertyMapEntry aURLPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_ANCTYPE),  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_ANCTYPES), 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_REPR),     0,  cppu::UnoType<OUString>::get(),    0, 0},
        {OUString(SC_UNONAME_TARGET),   0,  cppu::UnoType<OUString>::get(),    0, 0},
        {OUString(SC_UNONAME_TEXTWRAP), 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_URL),      0,  cppu::UnoType<OUString>::get(),    0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aURLPropertySet_Impl( aURLPropertyMap_Impl );
    return &aURLPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetHeaderFieldPropertySet()
{
    static const SfxItemPropertyMapEntry aHeaderFieldPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_ANCTYPE),  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_ANCTYPES), 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_TEXTWRAP), 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aHeaderFieldPropertySet_Impl( aHeaderFieldPropertyMap_Impl );
    return &aHeaderFieldPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetFileFieldPropertySet()
{
    static const SfxItemPropertyMapEntry aFileFieldPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_ANCTYPE),  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_ANCTYPES), 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_FILEFORM), 0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        {OUString(SC_UNONAME_TEXTWRAP), 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aFileFieldPropertySet_Impl( aFileFieldPropertyMap_Impl );
    return &aFileFieldPropertySet_Impl;
}

SvxFileFormat lcl_UnoToSvxFileFormat( sal_Int16 nUnoValue )
{
    switch( nUnoValue )
    {
        case text::FilenameDisplayFormat::FULL: return SVXFILEFORMAT_FULLPATH;
        case text::FilenameDisplayFormat::PATH: return SVXFILEFORMAT_PATH;
        case text::FilenameDisplayFormat::NAME: return SVXFILEFORMAT_NAME;
        default:
            return SVXFILEFORMAT_NAME_EXT;
    }
}

sal_Int16 lcl_SvxToUnoFileFormat( SvxFileFormat nSvxValue )
{
    switch( nSvxValue )
    {
        case SVXFILEFORMAT_NAME_EXT:    return text::FilenameDisplayFormat::NAME_AND_EXT;
        case SVXFILEFORMAT_FULLPATH:    return text::FilenameDisplayFormat::FULL;
        case SVXFILEFORMAT_PATH:        return text::FilenameDisplayFormat::PATH;
        default:
            return text::FilenameDisplayFormat::NAME;
    }
}

}

SC_SIMPLE_SERVICE_INFO( ScCellFieldsObj, "ScCellFieldsObj", "com.sun.star.text.TextFields" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFieldsObj, "ScHeaderFieldsObj", "com.sun.star.text.TextFields" )

//  ScUnoEditEngine nur um aus einer EditEngine die Felder herauszubekommen...

enum ScUnoCollectMode
{
    SC_UNO_COLLECT_NONE,
    SC_UNO_COLLECT_COUNT,
    SC_UNO_COLLECT_FINDINDEX,
    SC_UNO_COLLECT_FINDPOS
};

/**
 * This class exists solely to allow searching through field items.  TODO:
 * Look into providing the same functionality directly in EditEngine, to
 * avoid having this class altogether.
 */
class ScUnoEditEngine : public ScEditEngineDefaulter
{
    ScUnoCollectMode    eMode;
    sal_uInt16              nFieldCount;
    sal_Int32           mnFieldType;
    SvxFieldData*       pFound;         // lokale Kopie
    sal_Int32           nFieldPar;
    sal_Int32           nFieldPos;
    sal_uInt16              nFieldIndex;

public:
    explicit ScUnoEditEngine(ScEditEngineDefaulter* pSource);
    virtual ~ScUnoEditEngine();

    virtual OUString  CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos,
                                    Color*& rTxtColor, Color*& rFldColor ) override;

    sal_uInt16 CountFields();
    SvxFieldData* FindByIndex(sal_uInt16 nIndex);
    SvxFieldData* FindByPos(sal_Int32 nPar, sal_Int32 nPos, sal_Int32 nType);

    sal_Int32       GetFieldPar() const     { return nFieldPar; }
    sal_Int32       GetFieldPos() const     { return nFieldPos; }
};

ScUnoEditEngine::ScUnoEditEngine(ScEditEngineDefaulter* pSource)
    : ScEditEngineDefaulter(*pSource)
    , eMode(SC_UNO_COLLECT_NONE)
    , nFieldCount(0)
    , mnFieldType(text::textfield::Type::UNSPECIFIED)
    , pFound(nullptr)
    , nFieldPar(0)
    , nFieldPos(0)
    , nFieldIndex(0)
{
    EditTextObject* pData = pSource->CreateTextObject();
    SetText( *pData );
    delete pData;
}

ScUnoEditEngine::~ScUnoEditEngine()
{
    delete pFound;
}

OUString ScUnoEditEngine::CalcFieldValue( const SvxFieldItem& rField,
            sal_Int32 nPara, sal_Int32 nPos, Color*& rTxtColor, Color*& rFldColor )
{
    OUString aRet(EditEngine::CalcFieldValue( rField, nPara, nPos, rTxtColor, rFldColor ));
    if (eMode != SC_UNO_COLLECT_NONE)
    {
        const SvxFieldData* pFieldData = rField.GetField();
        if ( pFieldData )
        {
            if (mnFieldType == text::textfield::Type::UNSPECIFIED || pFieldData->GetClassId() == mnFieldType)
            {
                if ( eMode == SC_UNO_COLLECT_FINDINDEX && !pFound && nFieldCount == nFieldIndex )
                {
                    pFound = pFieldData->Clone();
                    nFieldPar = nPara;
                    nFieldPos = nPos;
                }
                if ( eMode == SC_UNO_COLLECT_FINDPOS && !pFound &&
                        nPara == nFieldPar && nPos == nFieldPos )
                {
                    pFound = pFieldData->Clone();
                    nFieldIndex = nFieldCount;
                }
                ++nFieldCount;
            }
        }
    }
    return aRet;
}

sal_uInt16 ScUnoEditEngine::CountFields()
{
    eMode = SC_UNO_COLLECT_COUNT;
    mnFieldType = text::textfield::Type::UNSPECIFIED;
    nFieldCount = 0;
    UpdateFields();
    eMode = SC_UNO_COLLECT_NONE;

    return nFieldCount;
}

SvxFieldData* ScUnoEditEngine::FindByIndex(sal_uInt16 nIndex)
{
    eMode = SC_UNO_COLLECT_FINDINDEX;
    nFieldIndex = nIndex;
    mnFieldType = text::textfield::Type::UNSPECIFIED;
    nFieldCount = 0;
    UpdateFields();
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

SvxFieldData* ScUnoEditEngine::FindByPos(sal_Int32 nPar, sal_Int32 nPos, sal_Int32 nType)
{
    eMode = SC_UNO_COLLECT_FINDPOS;
    nFieldPar = nPar;
    nFieldPos = nPos;
    mnFieldType = nType;
    nFieldCount = 0;
    UpdateFields();
    mnFieldType = text::textfield::Type::UNSPECIFIED;
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

ScCellFieldsObj::ScCellFieldsObj(
    const uno::Reference<text::XTextRange>& xContent,
    ScDocShell* pDocSh, const ScAddress& rPos) :
    mxContent(xContent),
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    mpRefreshListeners( nullptr )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    mpEditSource = new ScCellEditSource( pDocShell, aCellPos );
}

ScCellFieldsObj::~ScCellFieldsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);

    delete mpEditSource;

    // increment refcount to prevent double call off dtor
    osl_atomic_increment( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
        if (mpRefreshListeners)
        {
            mpRefreshListeners->disposeAndClear(aEvent);
            DELETEZ( mpRefreshListeners );
        }
    }
}

void ScCellFieldsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! Ref-Update
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) &&
            static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }

    //  EditSource hat sich selber als Listener angemeldet
}

// XIndexAccess (via XTextFields)

uno::Reference<text::XTextField> ScCellFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);
    SvxFieldData* pData = aTempEngine.FindByIndex(static_cast<sal_uInt16>(Index));
    if (!pData)
        return uno::Reference<text::XTextField>();

    sal_Int32 nPar = aTempEngine.GetFieldPar();
    sal_Int32 nPos = aTempEngine.GetFieldPos();
    ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Feld ist 1 Zeichen

    sal_Int32 eType = pData->GetClassId();
    uno::Reference<text::XTextField> xRet(
        new ScEditFieldObj(mxContent, new ScCellEditSource(pDocShell, aCellPos), eType, aSelection));
    return xRet;
}

sal_Int32 SAL_CALL ScCellFieldsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    return aTempEngine.CountFields();       // Felder zaehlen, in Zelle ist der Typ egal
}

uno::Any SAL_CALL ScCellFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScCellFieldsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<text::XTextField>::get();
}

sal_Bool SAL_CALL ScCellFieldsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.text.TextFieldEnumeration"));
}

void SAL_CALL ScCellFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScCellFieldsObj::refresh(  )
                                    throw (uno::RuntimeException, std::exception)
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners(mpRefreshListeners->getElements());
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                lang::EventObject aEvent;
                aEvent.Source.set(uno::Reference< util::XRefreshable >(this));
                sal_uInt32 i(0);
                while (i < nLength)
                {
                    try
                    {
                        while(i < nLength)
                        {
                            static_cast< util::XRefreshListener* >(pInterfaces->get())->refreshed(aEvent);
                            ++pInterfaces;
                            ++i;
                        }
                    }
                    catch(uno::RuntimeException&)
                    {
                        ++pInterfaces;
                        ++i;
                    }
                }
            }
        }
    }
}

void SAL_CALL ScCellFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException, std::exception)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners = new cppu::OInterfaceContainerHelper(aMutex);
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScCellFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException, std::exception)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

ScHeaderFieldsObj::ScHeaderFieldsObj(ScHeaderFooterTextData& rData) :
    mrData(rData),
    mpRefreshListeners( nullptr )
{
    mpEditSource = new ScHeaderFooterEditSource(rData);
}

ScHeaderFieldsObj::~ScHeaderFieldsObj()
{
    delete mpEditSource;

    // increment refcount to prevent double call off dtor
    osl_atomic_increment( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        if (mpRefreshListeners)
        {
            mpRefreshListeners->disposeAndClear(aEvent);
            DELETEZ( mpRefreshListeners );
        }
    }
}

// XIndexAccess (via XTextFields)

uno::Reference<text::XTextField> ScHeaderFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    SvxFieldData* pData = aTempEngine.FindByIndex(static_cast<sal_uInt16>(Index));
    if (!pData)
        return nullptr;

    // Get the parent text range instance.
    uno::Reference<text::XTextRange> xTextRange;
    rtl::Reference<ScHeaderFooterContentObj> rContentObj = mrData.GetContentObj();
    uno::Reference<text::XText> xText;
    sal_uInt16 nPart = mrData.GetPart();
    if (nPart == SC_HDFT_LEFT)
        xText = rContentObj->getLeftText();
    else if (nPart == SC_HDFT_CENTER)
        xText = rContentObj->getCenterText();
    else
        xText = rContentObj->getRightText();

    uno::Reference<text::XTextRange> xTemp(xText, uno::UNO_QUERY);
    xTextRange = xTemp;

    sal_Int32 nPar = aTempEngine.GetFieldPar();
    sal_Int32 nPos = aTempEngine.GetFieldPos();
    ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Field is 1 character

    sal_Int32 eRealType = pData->GetClassId();
    uno::Reference<text::XTextField> xRet(
        new ScEditFieldObj(xTextRange, new ScHeaderFooterEditSource(mrData), eRealType, aSelection));
    return xRet;
}

sal_Int32 SAL_CALL ScHeaderFieldsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);
    return aTempEngine.CountFields();
}

uno::Any SAL_CALL ScHeaderFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScHeaderFieldsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<text::XTextField>::get();
}

sal_Bool SAL_CALL ScHeaderFieldsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.text.TextFieldEnumeration"));
}

void SAL_CALL ScHeaderFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScHeaderFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScHeaderFieldsObj::refresh(  )
                                    throw (uno::RuntimeException, std::exception)
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners(mpRefreshListeners->getElements());
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                lang::EventObject aEvent;
                aEvent.Source.set(uno::Reference< util::XRefreshable >(this));
                sal_uInt32 i(0);
                while (i < nLength)
                {
                    try
                    {
                        while(i < nLength)
                        {
                            static_cast< util::XRefreshListener* >(pInterfaces->get())->refreshed(aEvent);
                            ++pInterfaces;
                            ++i;
                        }
                    }
                    catch(uno::RuntimeException&)
                    {
                        ++pInterfaces;
                        ++i;
                    }
                }
            }
        }
    }
}

void SAL_CALL ScHeaderFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException, std::exception)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners = new cppu::OInterfaceContainerHelper(aMutex);
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScHeaderFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException, std::exception)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

SvxFieldData* ScEditFieldObj::getData()
{
    if (!mpData)
    {
        switch (meType)
        {
            case text::textfield::Type::DATE:
                mpData.reset(new SvxDateField);
            break;
            case text::textfield::Type::EXTENDED_FILE:
                mpData.reset(
                    new SvxExtFileField(OUString(), SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT));
            break;
            case text::textfield::Type::PAGE:
                mpData.reset(new SvxPageField);
            break;
            case text::textfield::Type::PAGES:
                mpData.reset(new SvxPagesField);
            break;
            case text::textfield::Type::TABLE:
                mpData.reset(new SvxTableField);
            break;
            case text::textfield::Type::TIME:
                mpData.reset(new SvxTimeField);
            break;
            case text::textfield::Type::EXTENDED_TIME:
            {
                if (mbIsDate)
                    mpData.reset(new SvxDateField);
                else
                    mpData.reset(new SvxExtTimeField);
            }
            break;
            case text::textfield::Type::DOCINFO_TITLE:
                mpData.reset(new SvxFileField);
            break;
            case text::textfield::Type::URL:
                mpData.reset(
                    new SvxURLField(OUString(), OUString(), SVXURLFORMAT_APPDEFAULT));
            break;
            default:
                mpData.reset(new SvxFieldData);
        }
    }
    return mpData.get();
}

void ScEditFieldObj::setPropertyValueURL(const OUString& rName, const css::uno::Any& rVal)
{
    OUString aStrVal;
    if (mpEditSource)
    {
        // Edit engine instance already exists for this field item.  Use it.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"setPropertyValue: Feld nicht gefunden");
        if (!pField)
            return;

        if (pField->GetClassId() != text::textfield::Type::URL)
            // Make sure this is indeed a URL field.
            return;

        SvxURLField* pURL = static_cast<SvxURLField*>(pField);

        if (rName == SC_UNONAME_URL)
        {
            if (rVal >>= aStrVal)
                pURL->SetURL(aStrVal);
        }
        else if (rName == SC_UNONAME_REPR)
        {
            if (rVal >>= aStrVal)
                pURL->SetRepresentation(aStrVal);
        }
        else if (rName == SC_UNONAME_TARGET)
        {
            if (rVal >>= aStrVal)
                pURL->SetTargetFrame(aStrVal);
        }
        else
            throw beans::UnknownPropertyException();

        pEditEngine->QuickInsertField( SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection );
        mpEditSource->UpdateData();
        return;
    }

    // Edit engine instance not yet present.  Store the item data for later use.
    SvxFieldData* pData = getData();
    if (!pData)
        throw uno::RuntimeException();

    SvxURLField* p = static_cast<SvxURLField*>(pData);
    if (rName == SC_UNONAME_URL)
    {
        if (rVal >>= aStrVal)
            p->SetURL(aStrVal);
    }
    else if (rName == SC_UNONAME_REPR)
    {
        if (rVal >>= aStrVal)
            p->SetRepresentation(aStrVal);
    }
    else if (rName == SC_UNONAME_TARGET)
    {
        if (rVal >>= aStrVal)
            p->SetTargetFrame(aStrVal);
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any ScEditFieldObj::getPropertyValueURL(const OUString& rName)
{
    uno::Any aRet;

    // anchor type is always "as character", text wrap always "none"

    if (mpEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        const SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"getPropertyValue: Feld nicht gefunden");
        if (!pField)
            throw uno::RuntimeException();

        if (pField->GetClassId() != text::textfield::Type::URL)
            throw uno::RuntimeException();

        const SvxURLField* pURL = static_cast<const SvxURLField*>(pField);

        if (rName == SC_UNONAME_URL)
            aRet <<= pURL->GetURL();
        else if (rName == SC_UNONAME_REPR)
            aRet <<= pURL->GetRepresentation();
        else if (rName == SC_UNONAME_TARGET)
            aRet <<= pURL->GetTargetFrame();
        else
            throw beans::UnknownPropertyException();
    }
    else        // noch nicht eingefuegt
    {
        const SvxFieldData* pField = getData();
        if (!pField)
            return aRet;

        const SvxURLField* pURL = static_cast<const SvxURLField*>(pField);
        if (rName == SC_UNONAME_URL)
            aRet <<= pURL->GetURL();
        else if (rName == SC_UNONAME_REPR)
            aRet <<= pURL->GetRepresentation();
        else if (rName == SC_UNONAME_TARGET)
            aRet <<= pURL->GetTargetFrame();
        else
            throw beans::UnknownPropertyException();
    }
    return aRet;
}

void ScEditFieldObj::setPropertyValueFile(const OUString& rName, const uno::Any& rVal)
{
    if (rName == SC_UNONAME_FILEFORM)
    {
        sal_Int16 nIntVal = 0;
        if (rVal >>= nIntVal)
        {
            SvxFileFormat eFormat = lcl_UnoToSvxFileFormat(nIntVal);
            if (mpEditSource)
            {
                ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
                ScUnoEditEngine aTempEngine(pEditEngine);
                SvxFieldData* pField = aTempEngine.FindByPos(
                        aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::EXTENDED_FILE);
                OSL_ENSURE(pField, "setPropertyValueFile: Field not found");
                if (pField)
                {
                    SvxExtFileField* pExtFile = static_cast<SvxExtFileField*>(pField);   // local to the ScUnoEditEngine
                    pExtFile->SetFormat(eFormat);
                    pEditEngine->QuickInsertField(SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection);
                    mpEditSource->UpdateData();
                }
            }
            else
            {
                SvxFieldData* pField = getData();
                SvxExtFileField* pExtFile = static_cast<SvxExtFileField*>(pField);
                pExtFile->SetFormat(eFormat);
            }
        }
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any ScEditFieldObj::getPropertyValueFile(const OUString& rName)
{
    uno::Any aRet;
    if (rName == SC_UNONAME_FILEFORM)
    {
        SvxFileFormat eFormat = SVXFILEFORMAT_NAME_EXT;
        const SvxFieldData* pField = nullptr;
        if (mpEditSource)
        {
            ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
            ScUnoEditEngine aTempEngine(pEditEngine);
            pField = aTempEngine.FindByPos(
                aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::EXTENDED_FILE);
        }
        else
            pField = getData();

        OSL_ENSURE(pField, "setPropertyValueFile: Field not found");
        if (!pField)
            throw uno::RuntimeException();

        const SvxExtFileField* pExtFile = static_cast<const SvxExtFileField*>(pField);
        eFormat = pExtFile->GetFormat();
        sal_Int16 nIntVal = lcl_SvxToUnoFileFormat(eFormat);
        aRet <<= nIntVal;
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

void ScEditFieldObj::setPropertyValueDateTime(const OUString& rName, const uno::Any& rVal)
{
    if (mpEditSource)
    {
        // Field already inserted.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);
        SvxFieldData* pField = aTempEngine.FindByPos(aSelection.nStartPara, aSelection.nStartPos, meType);
        if (!pField)
            return;

        switch (meType)
        {
            case text::textfield::Type::DATE:
            {
                SvxDateField* p = static_cast<SvxDateField*>(pField);
                if (rName == SC_UNONAME_ISDATE)
                {
                    // Do nothing for now.
                }
                else if (rName == SC_UNONAME_ISFIXED)
                {
                    SvxDateType eType = rVal.get<sal_Bool>() ? SVXDATETYPE_FIX : SVXDATETYPE_VAR;
                    p->SetType(eType);
                }
                else if (rName == SC_UNONAME_DATETIME)
                {
                    maDateTime = rVal.get<util::DateTime>();
                    Date aDate(maDateTime.Day, maDateTime.Month, maDateTime.Year);
                    p->SetFixDate(aDate);
                }
                else if (rName == SC_UNONAME_NUMFMT)
                {
                    mnNumFormat = rVal.get<sal_Int32>();
                    p->SetFormat(static_cast<SvxDateFormat>(mnNumFormat));
                }
                else
                    throw beans::UnknownPropertyException();
            }
            break;
            case text::textfield::Type::TIME:
            {
                // SvxTimeField doesn't have any attributes.
                if (rName != SC_UNONAME_ISDATE && rName != SC_UNONAME_ISFIXED &&
                    rName != SC_UNONAME_DATETIME && rName != SC_UNONAME_NUMFMT)
                    throw beans::UnknownPropertyException();
            }
            break;
            case text::textfield::Type::EXTENDED_TIME:
            {
                SvxExtTimeField* p = static_cast<SvxExtTimeField*>(pField);
                if (rName == SC_UNONAME_ISDATE)
                {
                    // Do nothing for now.
                }
                else if (rName == SC_UNONAME_ISFIXED)
                {
                    SvxTimeType eType = rVal.get<sal_Bool>() ? SVXTIMETYPE_FIX : SVXTIMETYPE_VAR;
                    p->SetType(eType);
                }
                else if (rName == SC_UNONAME_DATETIME)
                {
                    maDateTime = rVal.get<util::DateTime>();
                    tools::Time aTime(maDateTime);
                    p->SetFixTime(aTime);
                }
                else if (rName == SC_UNONAME_NUMFMT)
                {
                    mnNumFormat = rVal.get<sal_Int32>();
                    p->SetFormat(static_cast<SvxTimeFormat>(mnNumFormat));
                }
                else
                    throw beans::UnknownPropertyException();
            }
            break;
            default:
                throw beans::UnknownPropertyException();
        }
    }
    else
    {
        if (rName == SC_UNONAME_ISDATE)
            mbIsDate = rVal.get<sal_Bool>();
        else if (rName == SC_UNONAME_ISFIXED)
            mbIsFixed = rVal.get<sal_Bool>();
        else if (rName == SC_UNONAME_DATETIME)
            maDateTime = rVal.get<util::DateTime>();
        else if (rName == SC_UNONAME_NUMFMT)
            mnNumFormat = rVal.get<sal_Int32>();
        else
            throw beans::UnknownPropertyException();
    }
}

uno::Any ScEditFieldObj::getPropertyValueDateTime(const OUString& rName)
{
    if (mpEditSource)
    {
        // Field already inserted.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);
        SvxFieldData* pField = aTempEngine.FindByPos(aSelection.nStartPara, aSelection.nStartPos, meType);
        if (!pField)
            throw uno::RuntimeException();

        switch (meType)
        {
            case text::textfield::Type::DATE:
            {
                SvxDateField* p = static_cast<SvxDateField*>(pField);
                if (rName == SC_UNONAME_ISDATE)
                    return uno::makeAny(true);

                if (rName == SC_UNONAME_ISFIXED)
                    return uno::makeAny<sal_Bool>(p->GetType() == SVXDATETYPE_FIX);

                if (rName == SC_UNONAME_DATETIME)
                {
                    Date aD(p->GetFixDate());
                    maDateTime.Year = aD.GetYear();
                    maDateTime.Month = aD.GetMonth();
                    maDateTime.Day = aD.GetDay();
                    maDateTime.Hours = 0;
                    maDateTime.Minutes = 0;
                    maDateTime.Seconds = 0;
                    maDateTime.NanoSeconds = 0;
                    return uno::makeAny(maDateTime);
                }

                if (rName == SC_UNONAME_NUMFMT)
                    return uno::makeAny<sal_Int32>(p->GetFormat());
            }
            break;
            case text::textfield::Type::TIME:
            {
                // SvxTimeField doesn't have any attributes.
                if (rName == SC_UNONAME_ISDATE)
                    return uno::makeAny(false);

                if (rName == SC_UNONAME_ISFIXED)
                    return uno::makeAny(false);

                if (rName == SC_UNONAME_DATETIME)
                    // This is the best we can do.
                    return uno::makeAny(maDateTime);

                if (rName == SC_UNONAME_NUMFMT)
                    // Same as above.
                    return uno::makeAny<sal_Int32>(0);
            }
            break;
            case text::textfield::Type::EXTENDED_TIME:
            {
                SvxExtTimeField* p = static_cast<SvxExtTimeField*>(pField);
                if (rName == SC_UNONAME_ISDATE)
                    return uno::makeAny(false);

                if (rName == SC_UNONAME_ISFIXED)
                    return uno::makeAny<sal_Bool>(p->GetType() == SVXTIMETYPE_FIX);

                if (rName == SC_UNONAME_DATETIME)
                {
                    tools::Time aT(p->GetFixTime());
                    maDateTime.Year = 0;
                    maDateTime.Month = 0;
                    maDateTime.Day = 0;
                    maDateTime.Hours = aT.GetHour();
                    maDateTime.Minutes = aT.GetMin();
                    maDateTime.Seconds = aT.GetSec();
                    maDateTime.NanoSeconds = aT.GetNanoSec();
                    return uno::makeAny(maDateTime);
                }

                if (rName == SC_UNONAME_NUMFMT)
                    return uno::makeAny<sal_Int32>(p->GetFormat());
            }
            break;
            default:
                ;
        }
    }
    else
    {
        if (rName == SC_UNONAME_ISDATE)
            return uno::makeAny<sal_Bool>(mbIsDate);

        if (rName == SC_UNONAME_ISFIXED)
            return uno::makeAny<sal_Bool>(mbIsFixed);

        if (rName == SC_UNONAME_DATETIME)
            return uno::makeAny(maDateTime);

        if (rName == SC_UNONAME_NUMFMT)
            return uno::makeAny(mnNumFormat);
    }

    throw beans::UnknownPropertyException();
}

void ScEditFieldObj::setPropertyValueSheet(const OUString& rName, const uno::Any& rVal)
{
    if (mpEditSource)
    {
        // Edit engine instance already exists for this field item.  Use it.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"setPropertyValue: Feld nicht gefunden");
        if (!pField)
            return;

        if (pField->GetClassId() != text::textfield::Type::TABLE)
            // Make sure this is indeed a URL field.
            return;

        SvxTableField* p = static_cast<SvxTableField*>(pField);

        if (rName == SC_UNONAME_TABLEPOS)
        {
            sal_Int32 nTab = rVal.get<sal_Int32>();
            p->SetTab(nTab);
        }
        else
            throw beans::UnknownPropertyException();

        pEditEngine->QuickInsertField(SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection);
        mpEditSource->UpdateData();
        return;
    }

    // Edit engine instance not yet present.  Store the item data for later use.
    SvxFieldData* pData = getData();
    if (!pData)
        throw uno::RuntimeException();

    SvxTableField* p = static_cast<SvxTableField*>(pData);
    if (rName == SC_UNONAME_TABLEPOS)
    {
        sal_Int32 nTab = rVal.get<sal_Int32>();
        p->SetTab(nTab);
    }
    else
        throw beans::UnknownPropertyException();

}

ScEditFieldObj::ScEditFieldObj(
    const uno::Reference<text::XTextRange>& rContent,
    ScEditSource* pEditSrc, sal_Int32 eType, const ESelection& rSel) :
    OComponentHelper(getMutex()),
    pPropSet(nullptr),
    mpEditSource(pEditSrc),
    aSelection(rSel),
    meType(eType), mpData(nullptr), mpContent(rContent), mnNumFormat(0), mbIsDate(false), mbIsFixed(false)
{
    switch (meType)
    {
        case text::textfield::Type::DOCINFO_TITLE:
            pPropSet = getEmptyPropertySet();
        break;
        case text::textfield::Type::EXTENDED_FILE:
            pPropSet = lcl_GetFileFieldPropertySet();
        break;
        case text::textfield::Type::URL:
            pPropSet = lcl_GetURLPropertySet();
        break;
        case text::textfield::Type::DATE:
        case text::textfield::Type::TIME:
        case text::textfield::Type::EXTENDED_TIME:
            pPropSet = getDateTimePropertySet();
        break;
        default:
            pPropSet = lcl_GetHeaderFieldPropertySet();
    }

    if (meType == text::textfield::Type::DATE)
        mbIsDate = true;
}

void ScEditFieldObj::InitDoc(
    const uno::Reference<text::XTextRange>& rContent, ScEditSource* pEditSrc, const ESelection& rSel)
{
    if (!mpEditSource)
    {
        mpContent = rContent;
        mpData.reset();

        aSelection = rSel;
        mpEditSource = pEditSrc;
    }
}

ScEditFieldObj::~ScEditFieldObj()
{
    delete mpEditSource;
}

SvxFieldItem ScEditFieldObj::CreateFieldItem()
{
    OSL_ENSURE( !mpEditSource, "CreateFieldItem mit eingefuegtem Feld" );
    return SvxFieldItem(*getData(), EE_FEATURE_FIELD);
}

void ScEditFieldObj::DeleteField()
{
    if (mpEditSource)
    {
        SvxTextForwarder* pForwarder = mpEditSource->GetTextForwarder();
        pForwarder->QuickInsertText( OUString(), aSelection );
        mpEditSource->UpdateData();

        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos  = aSelection.nStartPos;

        //! Broadcast, um Selektion in anderen Objekten anzupassen
        //! (auch bei anderen Aktionen)
    }
}

bool ScEditFieldObj::IsInserted() const
{
    return mpEditSource != nullptr;
}

// XTextField

OUString SAL_CALL ScEditFieldObj::getPresentation( sal_Bool bShowCommand )
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!mpEditSource)
        return OUString();

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    //  Typ egal (in Zellen gibts nur URLs)
    const SvxFieldData* pField = aTempEngine.FindByPos(
        aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
    OSL_ENSURE(pField,"getPresentation: Feld nicht gefunden");
    if (!pField)
        return OUString();

    switch (meType)
    {
        case text::textfield::Type::URL:
        {
            if (pField->GetClassId() != text::textfield::Type::URL)
                // Not an URL field, but URL is expected.
                throw uno::RuntimeException();

            const SvxURLField* pURL = static_cast<const SvxURLField*>(pField);
            return bShowCommand ? pURL->GetURL() : pURL->GetRepresentation();
        }
        break;
        default:
            ;
    }
    return OUString();
}

// XTextContent

void SAL_CALL ScEditFieldObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (xTextRange.is())
    {
        uno::Reference<text::XText> xText(xTextRange->getText());
        if (xText.is())
        {
            xText->insertTextContent( xTextRange, this, true );
        }
    }
}

uno::Reference<text::XTextRange> SAL_CALL ScEditFieldObj::getAnchor() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return mpContent;
}

// XComponent

void SAL_CALL ScEditFieldObj::dispose() throw(uno::RuntimeException, std::exception)
{
    OComponentHelper::dispose();
}

void SAL_CALL ScEditFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException, std::exception)
{
    OComponentHelper::addEventListener( xListener );
}

void SAL_CALL ScEditFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException, std::exception)
{
    OComponentHelper::removeEventListener( xListener );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScEditFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<beans::XPropertySetInfo> aRef = pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL ScEditFieldObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw (beans::UnknownPropertyException, beans::PropertyVetoException,
                       lang::IllegalArgumentException, lang::WrappedTargetException,
                       uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (aPropertyName == SC_UNONAME_ANCHOR)
    {
        aValue >>= mpContent;
        return;
    }

    switch (meType)
    {
        case text::textfield::Type::URL:
            setPropertyValueURL(aPropertyName, aValue);
        break;
        case text::textfield::Type::EXTENDED_FILE:
            setPropertyValueFile(aPropertyName, aValue);
        break;
        case text::textfield::Type::DATE:
        case text::textfield::Type::TIME:
        case text::textfield::Type::EXTENDED_TIME:
            setPropertyValueDateTime(aPropertyName, aValue);
        break;
        case text::textfield::Type::TABLE:
            setPropertyValueSheet(aPropertyName, aValue);
        break;
        case text::textfield::Type::DOCINFO_TITLE:
        default:
            throw beans::UnknownPropertyException();
    }
}

uno::Any SAL_CALL ScEditFieldObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (aPropertyName == SC_UNONAME_TEXTFIELD_TYPE)
        return uno::makeAny(meType);

    if (aPropertyName == SC_UNONAME_ANCHOR)
        return uno::makeAny(mpContent);

    if (aPropertyName == SC_UNONAME_ANCTYPE)
    {
        uno::Any aRet;
        aRet <<= text::TextContentAnchorType_AS_CHARACTER;
        return aRet;
    }
    if (aPropertyName == SC_UNONAME_ANCTYPES)
    {
        uno::Any aRet;
        uno::Sequence<text::TextContentAnchorType> aSeq(1);
        aSeq[0] = text::TextContentAnchorType_AS_CHARACTER;
        aRet <<= aSeq;
        return aRet;
    }
    if (aPropertyName == SC_UNONAME_TEXTWRAP)
    {
        uno::Any aRet;
        aRet <<= text::WrapTextMode_NONE;
        return aRet;
    }

    switch (meType)
    {
        case text::textfield::Type::URL:
            return getPropertyValueURL(aPropertyName);
        case text::textfield::Type::EXTENDED_FILE:
            return getPropertyValueFile(aPropertyName);
        case text::textfield::Type::DATE:
        case text::textfield::Type::TIME:
        case text::textfield::Type::EXTENDED_TIME:
            return getPropertyValueDateTime(aPropertyName);
        case text::textfield::Type::DOCINFO_TITLE:
        default:
            throw beans::UnknownPropertyException();
    }
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScEditFieldObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScEditFieldObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScEditFieldObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScEditFieldObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScEditFieldObj::getUnoTunnelId()
{
    return theScEditFieldObjUnoTunnelId::get().getSeq();
}

ScEditFieldObj* ScEditFieldObj::getImplementation(const uno::Reference<text::XTextContent>& xObj)
{
    ScEditFieldObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScEditFieldObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

// XServiceInfo

OUString SAL_CALL ScEditFieldObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("ScEditFieldObj");
}

sal_Bool SAL_CALL ScEditFieldObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScEditFieldObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextField";
    pArray[1] = "com.sun.star.text.TextContent";
    return aRet;
}

uno::Sequence<uno::Type> SAL_CALL ScEditFieldObj::getTypes() throw(uno::RuntimeException, std::exception)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(OComponentHelper::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 4 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = cppu::UnoType<text::XTextField>::get();
        pPtr[nParentLen + 1] = cppu::UnoType<beans::XPropertySet>::get();
        pPtr[nParentLen + 2] = cppu::UnoType<lang::XUnoTunnel>::get();
        pPtr[nParentLen + 3] = cppu::UnoType<lang::XServiceInfo>::get();

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScEditFieldObj::getImplementationId()
                                                    throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
