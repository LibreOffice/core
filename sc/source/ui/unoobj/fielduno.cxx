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

#include <memory>
#include <fielduno.hxx>
#include <textuno.hxx>
#include <miscuno.hxx>
#include <docsh.hxx>
#include <hints.hxx>
#include <editsrc.hxx>
#include <unonames.hxx>
#include <editutil.hxx>

#include <svl/hint.hxx>
#include <vcl/svapp.hxx>

#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace {

//  no Which-ID here, map only for PropertySetInfo

const SfxItemPropertySet* getDateTimePropertySet()
{
    static const SfxItemPropertyMapEntry aMapContent[] =
    {
        { u"" SC_UNONAME_DATETIME, 0, cppu::UnoType<util::DateTime>::get(), 0, 0 },
        { u"" SC_UNONAME_ISFIXED,  0, cppu::UnoType<bool>::get(),                  0, 0 },
        { u"" SC_UNONAME_ISDATE,   0, cppu::UnoType<bool>::get(),                  0, 0 },
        { u"" SC_UNONAME_NUMFMT,   0, cppu::UnoType<sal_Int32>::get(),      0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aMap(aMapContent);
    return &aMap;
}

const SfxItemPropertySet* getEmptyPropertySet()
{
    static const SfxItemPropertyMapEntry aMapContent[] =
    {
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aMap(aMapContent);
    return &aMap;
}

const SfxItemPropertySet* lcl_GetURLPropertySet()
{
    static const SfxItemPropertyMapEntry aURLPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ANCTYPE,  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ANCTYPES, 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_REPR,     0,  cppu::UnoType<OUString>::get(),    0, 0},
        {u"" SC_UNONAME_TARGET,   0,  cppu::UnoType<OUString>::get(),    0, 0},
        {u"" SC_UNONAME_TEXTWRAP, 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_URL,      0,  cppu::UnoType<OUString>::get(),    0, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aURLPropertySet_Impl( aURLPropertyMap_Impl );
    return &aURLPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetHeaderFieldPropertySet()
{
    static const SfxItemPropertyMapEntry aHeaderFieldPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ANCTYPE,  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ANCTYPES, 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TEXTWRAP, 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aHeaderFieldPropertySet_Impl( aHeaderFieldPropertyMap_Impl );
    return &aHeaderFieldPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetFileFieldPropertySet()
{
    static const SfxItemPropertyMapEntry aFileFieldPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ANCTYPE,  0,  cppu::UnoType<text::TextContentAnchorType>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ANCTYPES, 0,  cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_FILEFORM, 0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        {u"" SC_UNONAME_TEXTWRAP, 0,  cppu::UnoType<text::WrapTextMode>::get(), beans::PropertyAttribute::READONLY, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aFileFieldPropertySet_Impl( aFileFieldPropertyMap_Impl );
    return &aFileFieldPropertySet_Impl;
}

SvxFileFormat lcl_UnoToSvxFileFormat( sal_Int16 nUnoValue )
{
    switch( nUnoValue )
    {
        case text::FilenameDisplayFormat::FULL: return SvxFileFormat::PathFull;
        case text::FilenameDisplayFormat::PATH: return SvxFileFormat::PathOnly;
        case text::FilenameDisplayFormat::NAME: return SvxFileFormat::NameOnly;
        default:
            return SvxFileFormat::NameAndExt;
    }
}

sal_Int16 lcl_SvxToUnoFileFormat( SvxFileFormat nSvxValue )
{
    switch( nSvxValue )
    {
        case SvxFileFormat::NameAndExt:    return text::FilenameDisplayFormat::NAME_AND_EXT;
        case SvxFileFormat::PathFull:    return text::FilenameDisplayFormat::FULL;
        case SvxFileFormat::PathOnly:        return text::FilenameDisplayFormat::PATH;
        default:
            return text::FilenameDisplayFormat::NAME;
    }
}

}

SC_SIMPLE_SERVICE_INFO( ScCellFieldsObj, "ScCellFieldsObj", "com.sun.star.text.TextFields" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFieldsObj, "ScHeaderFieldsObj", "com.sun.star.text.TextFields" )

namespace {

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
    sal_uInt16          nFieldCount;
    sal_Int32           mnFieldType;
    std::unique_ptr<SvxFieldData>
                        pFound;         // local copy
    sal_Int32           nFieldPar;
    sal_Int32           nFieldPos;
    sal_uInt16          nFieldIndex;

public:
    explicit ScUnoEditEngine(ScEditEngineDefaulter* pSource);

    virtual OUString  CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos,
                                   std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor ) override;

    sal_uInt16 CountFields();
    SvxFieldData* FindByIndex(sal_uInt16 nIndex);
    SvxFieldData* FindByPos(sal_Int32 nPar, sal_Int32 nPos, sal_Int32 nType);

    sal_Int32       GetFieldPar() const     { return nFieldPar; }
    sal_Int32       GetFieldPos() const     { return nFieldPos; }
};

}

ScUnoEditEngine::ScUnoEditEngine(ScEditEngineDefaulter* pSource)
    : ScEditEngineDefaulter(*pSource)
    , eMode(SC_UNO_COLLECT_NONE)
    , nFieldCount(0)
    , mnFieldType(text::textfield::Type::UNSPECIFIED)
    , nFieldPar(0)
    , nFieldPos(0)
    , nFieldIndex(0)
{
    std::unique_ptr<EditTextObject> pData = pSource->CreateTextObject();
    SetTextCurrentDefaults( *pData );
}

OUString ScUnoEditEngine::CalcFieldValue( const SvxFieldItem& rField,
            sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor )
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

    return pFound.get();
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

    return pFound.get();
}

ScCellFieldsObj::ScCellFieldsObj(
    const uno::Reference<text::XTextRange>& xContent,
    ScDocShell* pDocSh, const ScAddress& rPos) :
    mxContent(xContent),
    pDocShell( pDocSh ),
    aCellPos( rPos )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    mpEditSource.reset( new ScCellEditSource( pDocShell, aCellPos ) );
}

ScCellFieldsObj::~ScCellFieldsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);

    mpEditSource.reset();

    // increment refcount to prevent double call off dtor
    osl_atomic_increment( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
        mpRefreshListeners->disposeAndClear(aEvent);
        mpRefreshListeners.reset();
    }
}

void ScCellFieldsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! update of references
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }

    //  EditSource registered itself as a listener
}

// XIndexAccess (via XTextFields)

uno::Reference<text::XTextField> ScCellFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Field functions have to be passed to the forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);
    SvxFieldData* pData = aTempEngine.FindByIndex(static_cast<sal_uInt16>(Index));
    if (!pData)
        return uno::Reference<text::XTextField>();

    sal_Int32 nPar = aTempEngine.GetFieldPar();
    sal_Int32 nPos = aTempEngine.GetFieldPos();
    ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Field size is 1 character

    sal_Int32 eType = pData->GetClassId();
    uno::Reference<text::XTextField> xRet(
        new ScEditFieldObj(mxContent, std::make_unique<ScCellEditSource>(pDocShell, aCellPos), eType, aSelection));
    return xRet;
}

sal_Int32 SAL_CALL ScCellFieldsObj::getCount()
{
    SolarMutexGuard aGuard;

    //! Field functions have to be passed to the forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    return aTempEngine.CountFields();       // count the fields, we don't care about their type in the cell
}

uno::Any SAL_CALL ScCellFieldsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (!xField.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xField);
}

uno::Type SAL_CALL ScCellFieldsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<text::XTextField>::get();
}

sal_Bool SAL_CALL ScCellFieldsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellFieldsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.text.TextFieldEnumeration");
}

void SAL_CALL ScCellFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScCellFieldsObj::refresh(  )
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        lang::EventObject aEvent;
        aEvent.Source.set(uno::Reference< util::XRefreshable >(this));
        mpRefreshListeners->notifyEach( &util::XRefreshListener::refreshed, aEvent );
    }
}

void SAL_CALL ScCellFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners.reset( new comphelper::OInterfaceContainerHelper2(aMutex) );
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScCellFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

ScHeaderFieldsObj::ScHeaderFieldsObj(ScHeaderFooterTextData& rData) :
    mrData(rData)
{
    mpEditSource.reset( new ScHeaderFooterEditSource(rData) );
}

ScHeaderFieldsObj::~ScHeaderFieldsObj()
{
    mpEditSource.reset();

    // increment refcount to prevent double call off dtor
    osl_atomic_increment( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        mpRefreshListeners->disposeAndClear(aEvent);
        mpRefreshListeners.reset();
    }
}

// XIndexAccess (via XTextFields)

uno::Reference<text::XTextField> ScHeaderFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Field functions have to be passed to the forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    SvxFieldData* pData = aTempEngine.FindByIndex(static_cast<sal_uInt16>(Index));
    if (!pData)
        return nullptr;

    // Get the parent text range instance.
    uno::Reference<text::XTextRange> xTextRange;
    uno::Reference<sheet::XHeaderFooterContent> xContentObj = mrData.GetContentObj();
    if (!xContentObj.is())
        throw uno::RuntimeException("");

    rtl::Reference<ScHeaderFooterContentObj> pContentObj = ScHeaderFooterContentObj::getImplementation(xContentObj);
    uno::Reference<text::XText> xText;

    switch ( mrData.GetPart() )
    {
        case ScHeaderFooterPart::LEFT:
            xText = pContentObj->getLeftText();
        break;
        case ScHeaderFooterPart::CENTER:
            xText = pContentObj->getCenterText();
        break;
        case ScHeaderFooterPart::RIGHT:
            xText = pContentObj->getRightText();
        break;
    }

    xTextRange = xText;

    sal_Int32 nPar = aTempEngine.GetFieldPar();
    sal_Int32 nPos = aTempEngine.GetFieldPos();
    ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Field size is 1 character

    sal_Int32 eRealType = pData->GetClassId();
    uno::Reference<text::XTextField> xRet(
        new ScEditFieldObj(xTextRange, std::make_unique<ScHeaderFooterEditSource>(mrData), eRealType, aSelection));
    return xRet;
}

sal_Int32 SAL_CALL ScHeaderFieldsObj::getCount()
{
    SolarMutexGuard aGuard;

    //! Field functions have to be passed to the forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);
    return aTempEngine.CountFields();
}

uno::Any SAL_CALL ScHeaderFieldsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (!xField.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xField);
}

uno::Type SAL_CALL ScHeaderFieldsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<text::XTextField>::get();
}

sal_Bool SAL_CALL ScHeaderFieldsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFieldsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.text.TextFieldEnumeration");
}

void SAL_CALL ScHeaderFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScHeaderFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScHeaderFieldsObj::refresh(  )
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        lang::EventObject aEvent;
        aEvent.Source.set(uno::Reference< util::XRefreshable >(this));
        mpRefreshListeners->notifyEach( &util::XRefreshListener::refreshed, aEvent);
    }
}

void SAL_CALL ScHeaderFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners.reset(new comphelper::OInterfaceContainerHelper2(aMutex));
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScHeaderFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

SvxFieldData& ScEditFieldObj::getData()
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
                    new SvxExtFileField(OUString(), SvxFileType::Var, SvxFileFormat::NameAndExt));
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
                    new SvxURLField(OUString(), OUString(), SvxURLFormat::AppDefault));
            break;
            default:
                mpData.reset(new SvxFieldData);
        }
    }
    return *mpData;
}

void ScEditFieldObj::setPropertyValueURL(const OUString& rName, const css::uno::Any& rVal)
{
    OUString aStrVal;
    if (mpEditSource)
    {
        // Edit engine instance already exists for this field item.  Use it.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  don't care about the type (only URLs can be found in the cells)
        SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"setPropertyValue: Field not found");
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
            throw beans::UnknownPropertyException(rName);

        pEditEngine->QuickInsertField( SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection );
        mpEditSource->UpdateData();
        return;
    }

    // Edit engine instance not yet present.  Store the item data for later use.
    SvxURLField& rData = static_cast<SvxURLField&>(getData());
    if (rName == SC_UNONAME_URL)
    {
        if (rVal >>= aStrVal)
            rData.SetURL(aStrVal);
    }
    else if (rName == SC_UNONAME_REPR)
    {
        if (rVal >>= aStrVal)
            rData.SetRepresentation(aStrVal);
    }
    else if (rName == SC_UNONAME_TARGET)
    {
        if (rVal >>= aStrVal)
            rData.SetTargetFrame(aStrVal);
    }
    else
        throw beans::UnknownPropertyException(rName);
}

uno::Any ScEditFieldObj::getPropertyValueURL(const OUString& rName)
{
    uno::Any aRet;

    // anchor type is always "as character", text wrap always "none"

    if (mpEditSource)
    {
        //! Field functions have to be passed to the forwarder !!!
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  don't care about the type (only URLs can be found in the cells)
        const SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"getPropertyValue: Field not found");
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
            throw beans::UnknownPropertyException(rName);
    }
    else        // not inserted yet
    {
        const SvxURLField& rURL = static_cast<const SvxURLField&>(getData());

        if (rName == SC_UNONAME_URL)
            aRet <<= rURL.GetURL();
        else if (rName == SC_UNONAME_REPR)
            aRet <<= rURL.GetRepresentation();
        else if (rName == SC_UNONAME_TARGET)
            aRet <<= rURL.GetTargetFrame();
        else
            throw beans::UnknownPropertyException(rName);
    }
    return aRet;
}

void ScEditFieldObj::setPropertyValueFile(const OUString& rName, const uno::Any& rVal)
{
    if (rName != SC_UNONAME_FILEFORM)
        throw beans::UnknownPropertyException(rName);

    sal_Int16 nIntVal = 0;
    if (!(rVal >>= nIntVal))
        return;

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
        SvxExtFileField& rExtFile = static_cast<SvxExtFileField&>(getData());
        rExtFile.SetFormat(eFormat);
    }

}

uno::Any ScEditFieldObj::getPropertyValueFile(const OUString& rName)
{
    uno::Any aRet;
    if (rName != SC_UNONAME_FILEFORM)
        throw beans::UnknownPropertyException(rName);

    SvxFileFormat eFormat = SvxFileFormat::NameAndExt;
    const SvxFieldData* pField = nullptr;
    if (mpEditSource)
    {
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);
        pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::EXTENDED_FILE);
    }
    else
        pField = &getData();

    OSL_ENSURE(pField, "setPropertyValueFile: Field not found");
    if (!pField)
        throw uno::RuntimeException();

    const SvxExtFileField* pExtFile = static_cast<const SvxExtFileField*>(pField);
    eFormat = pExtFile->GetFormat();
    sal_Int16 nIntVal = lcl_SvxToUnoFileFormat(eFormat);
    aRet <<= nIntVal;

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
                    SvxDateType eType = rVal.get<bool>() ? SvxDateType::Fix : SvxDateType::Var;
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
                    throw beans::UnknownPropertyException(rName);
            }
            break;
            case text::textfield::Type::TIME:
            {
                // SvxTimeField doesn't have any attributes.
                if (rName != SC_UNONAME_ISDATE && rName != SC_UNONAME_ISFIXED &&
                    rName != SC_UNONAME_DATETIME && rName != SC_UNONAME_NUMFMT)
                    throw beans::UnknownPropertyException(rName);
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
                    SvxTimeType eType = rVal.get<bool>() ? SvxTimeType::Fix : SvxTimeType::Var;
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
                    throw beans::UnknownPropertyException(rName);
            }
            break;
            default:
                throw beans::UnknownPropertyException(rName);
        }
    }
    else
    {
        if (rName == SC_UNONAME_ISDATE)
            mbIsDate = rVal.get<bool>();
        else if (rName == SC_UNONAME_ISFIXED)
            mbIsFixed = rVal.get<bool>();
        else if (rName == SC_UNONAME_DATETIME)
            maDateTime = rVal.get<util::DateTime>();
        else if (rName == SC_UNONAME_NUMFMT)
            mnNumFormat = rVal.get<sal_Int32>();
        else
            throw beans::UnknownPropertyException(rName);
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
                    return uno::makeAny<sal_Bool>(p->GetType() == SvxDateType::Fix);

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
                    return uno::makeAny<sal_Int32>(static_cast<sal_Int32>(p->GetFormat()));
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
                    return uno::makeAny<sal_Bool>(p->GetType() == SvxTimeType::Fix);

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
                    return uno::makeAny<sal_Int32>(static_cast<sal_Int32>(p->GetFormat()));
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

    throw beans::UnknownPropertyException(rName);
}

void ScEditFieldObj::setPropertyValueSheet(const OUString& rName, const uno::Any& rVal)
{
    if (mpEditSource)
    {
        // Edit engine instance already exists for this field item.  Use it.
        ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  don't care about the type (only URLs can be found in the cells)
        SvxFieldData* pField = aTempEngine.FindByPos(
            aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
        OSL_ENSURE(pField,"setPropertyValue: Field not found");
        if (!pField)
            return;

        if (pField->GetClassId() != text::textfield::Type::TABLE)
            // Make sure this is indeed a URL field.
            return;

        SvxTableField* p = static_cast<SvxTableField*>(pField);

        if (rName != SC_UNONAME_TABLEPOS)
            throw beans::UnknownPropertyException(rName);

        sal_Int32 nTab = rVal.get<sal_Int32>();
        p->SetTab(nTab);


        pEditEngine->QuickInsertField(SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection);
        mpEditSource->UpdateData();
        return;
    }

    // Edit engine instance not yet present.  Store the item data for later use.
    SvxTableField& r = static_cast<SvxTableField&>(getData());
    if (rName != SC_UNONAME_TABLEPOS)
        throw beans::UnknownPropertyException(rName);

    sal_Int32 nTab = rVal.get<sal_Int32>();
    r.SetTab(nTab);
}

ScEditFieldObj::ScEditFieldObj(
    const uno::Reference<text::XTextRange>& rContent,
    std::unique_ptr<ScEditSource> pEditSrc, sal_Int32 eType, const ESelection& rSel) :
    OComponentHelper(getMutex()),
    pPropSet(nullptr),
    mpEditSource(std::move(pEditSrc)),
    aSelection(rSel),
    meType(eType), mpContent(rContent), mnNumFormat(0), mbIsDate(false), mbIsFixed(false)
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
    const uno::Reference<text::XTextRange>& rContent, std::unique_ptr<ScEditSource> pEditSrc, const ESelection& rSel)
{
    if (!mpEditSource)
    {
        mpContent = rContent;
        mpData.reset();

        aSelection = rSel;
        mpEditSource = std::move( pEditSrc );
    }
}

ScEditFieldObj::~ScEditFieldObj()
{
}

SvxFieldItem ScEditFieldObj::CreateFieldItem()
{
    OSL_ENSURE( !mpEditSource, "CreateFieldItem with inserted field" );
    return SvxFieldItem(getData(), EE_FEATURE_FIELD);
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

        //! Broadcast in order to adjust selection in other objects
        //! (also for other actions)
    }
}

bool ScEditFieldObj::IsInserted() const
{
    return mpEditSource != nullptr;
}

// XTextField

OUString SAL_CALL ScEditFieldObj::getPresentation( sal_Bool bShowCommand )
{
    SolarMutexGuard aGuard;

    if (!mpEditSource)
        return OUString();

    //! Field functions have to be passed to the forwarder !!!
    ScEditEngineDefaulter* pEditEngine = mpEditSource->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    //  don't care about the type (only URLs can be found in the cells)
    const SvxFieldData* pField = aTempEngine.FindByPos(
        aSelection.nStartPara, aSelection.nStartPos, text::textfield::Type::UNSPECIFIED);
    OSL_ENSURE(pField,"getPresentation: Field not found");
    if (!pField)
        return OUString();

    switch (meType)
    {
        case text::textfield::Type::URL:
        {
            if (pField->GetClassId() != text::textfield::Type::URL)
                // Not a URL field, but URL is expected.
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

uno::Reference<text::XTextRange> SAL_CALL ScEditFieldObj::getAnchor()
{
    SolarMutexGuard aGuard;
    return mpContent;
}

// XComponent

void SAL_CALL ScEditFieldObj::dispose()
{
    OComponentHelper::dispose();
}

void SAL_CALL ScEditFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
{
    OComponentHelper::addEventListener( xListener );
}

void SAL_CALL ScEditFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
{
    OComponentHelper::removeEventListener( xListener );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScEditFieldObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    uno::Reference<beans::XPropertySetInfo> aRef = pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL ScEditFieldObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
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
            throw beans::UnknownPropertyException(OUString::number(meType));
    }
}

uno::Any SAL_CALL ScEditFieldObj::getPropertyValue( const OUString& aPropertyName )
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
            throw beans::UnknownPropertyException(OUString::number(meType));
    }
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScEditFieldObj )

// XUnoTunnel

UNO3_GETIMPLEMENTATION_IMPL(ScEditFieldObj);

// XServiceInfo

OUString SAL_CALL ScEditFieldObj::getImplementationName()
{
    return "ScEditFieldObj";
}

sal_Bool SAL_CALL ScEditFieldObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScEditFieldObj::getSupportedServiceNames()
{
    return {"com.sun.star.text.TextField",
            "com.sun.star.text.TextContent"};
}

uno::Sequence<uno::Type> SAL_CALL ScEditFieldObj::getTypes()
{
    return comphelper::concatSequences(
        OComponentHelper::getTypes(),
        uno::Sequence<uno::Type>
        {
            cppu::UnoType<text::XTextField>::get(),
            cppu::UnoType<beans::XPropertySet>::get(),
            cppu::UnoType<lang::XUnoTunnel>::get(),
            cppu::UnoType<lang::XServiceInfo>::get()
        } );
}

uno::Sequence<sal_Int8> SAL_CALL ScEditFieldObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
