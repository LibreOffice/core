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
#include "vbadocumentproperties.hxx"
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <ooo/vba/word/WdBuiltInProperty.hpp>
#include <ooo/vba/office/MsoDocProperties.hpp>
#include <tools/diagnose_ex.h>
#include <memory>
#include "wordvbahelper.hxx"
#include <fesh.hxx>
#include <docsh.hxx>
using namespace ::ooo::vba;
using namespace css;

/// @throws lang::IllegalArgumentException
static sal_Int8 lcl_toMSOPropType( const uno::Type& aType )
{
    sal_Int16 msoType = office::MsoDocProperties::msoPropertyTypeString;

    switch ( aType.getTypeClass() )
    {
        case uno::TypeClass_BOOLEAN:
            msoType =  office::MsoDocProperties::msoPropertyTypeBoolean;
            break;
        case uno::TypeClass_FLOAT:
            msoType =  office::MsoDocProperties::msoPropertyTypeFloat;
            break;
        case uno::TypeClass_STRUCT: // Assume date
            msoType =  office::MsoDocProperties::msoPropertyTypeDate;
            break;
        case  uno::TypeClass_BYTE:
        case  uno::TypeClass_SHORT:
        case  uno::TypeClass_LONG:
        case  uno::TypeClass_HYPER:
            msoType =  office::MsoDocProperties::msoPropertyTypeNumber;
            break;
        default:
            throw lang::IllegalArgumentException();
    }
    return msoType;
}

class PropertGetSetHelper
{
protected:
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference<document::XDocumentProperties> m_xDocProps;
public:
    explicit PropertGetSetHelper( const uno::Reference< frame::XModel >& xModel ):m_xModel( xModel )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> const
            xDocPropSupp(m_xModel, uno::UNO_QUERY_THROW);
        m_xDocProps.set(xDocPropSupp->getDocumentProperties(),
                uno::UNO_SET_THROW);
    }
    virtual ~PropertGetSetHelper() {}
    virtual uno::Any getPropertyValue( const OUString& rPropName ) = 0;
    virtual void setPropertyValue( const OUString& rPropName, const uno::Any& aValue ) = 0;
    uno::Reference< beans::XPropertySet > getUserDefinedProperties() {
        return uno::Reference<beans::XPropertySet>(
                m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    }

};

class BuiltinPropertyGetSetHelper : public PropertGetSetHelper
{
public:
    explicit BuiltinPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :PropertGetSetHelper( xModel )
    {
    }
    virtual uno::Any getPropertyValue( const OUString& rPropName ) override
    {
        if ( rPropName == "EditingDuration" )
        {
            sal_Int32 const nSecs = m_xDocProps->getEditingDuration();
            return uno::makeAny( nSecs/60 ); // minutes
        }
        else if ("Title" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getTitle());
        }
        else if ("Subject" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getSubject());
        }
        else if ("Author" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getAuthor());
        }
        else if ("Keywords" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getKeywords());
        }
        else if ("Description" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getDescription());
        }
        else if ("Template" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getTemplateName());
        }
        else if ("ModifiedBy" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getModifiedBy());
        }
        else if ("Generator" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getGenerator());
        }
        else if ("PrintDate" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getPrintDate());
        }
        else if ("CreationDate" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getCreationDate());
        }
        else if ("ModifyDate" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getModificationDate());
        }
        else if ("AutoloadURL" == rPropName)
        {
            return uno::makeAny(m_xDocProps->getAutoloadURL());
        }
        else
        {
            // fall back to user-defined properties
            return getUserDefinedProperties()->getPropertyValue(rPropName);
        }
    }
    virtual void setPropertyValue( const OUString& rPropName, const uno::Any& aValue ) override
    {
        if ("EditingDuration" == rPropName)
        {
            sal_Int32 nMins = 0;
            if (aValue >>= nMins)
            {
                m_xDocProps->setEditingDuration(nMins * 60); // convert minutes
            }
        }
        else if ("Title" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setTitle(str);
            }
        }
        else if ("Subject" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setSubject(str);
            }
        }
        else if ("Author" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setAuthor(str);
            }
        }
        else if ("Keywords" == rPropName)
        {
            uno::Sequence<OUString> keywords;
            if (aValue >>= keywords)
            {
                m_xDocProps->setKeywords(keywords);
            }
        }
        else if ("Description" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setDescription(str);
            }
        }
        else if ("Template" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setTemplateName(str);
            }
        }
        else if ("ModifiedBy" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setModifiedBy(str);
            }
        }
        else if ("Generator" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                return m_xDocProps->setGenerator(str);
            }
        }
        else if ("PrintDate" == rPropName)
        {
            util::DateTime dt;
            if (aValue >>= dt)
            {
                m_xDocProps->setPrintDate(dt);
            }
        }
        else if ("CreationDate" == rPropName)
        {
            util::DateTime dt;
            if (aValue >>= dt)
            {
                m_xDocProps->setCreationDate(dt);
            }
        }
        else if ("ModifyDate" == rPropName)
        {
            util::DateTime dt;
            if (aValue >>= dt)
            {
                m_xDocProps->setModificationDate(dt);
            }
        }
        else if ("AutoloadURL" == rPropName)
        {
            OUString str;
            if (aValue >>= str)
            {
                m_xDocProps->setAutoloadURL(str);
            }
        }
        else
        {
            // fall back to user-defined properties
            getUserDefinedProperties()->setPropertyValue(rPropName, aValue);
        }
    }
};

class CustomPropertyGetSetHelper : public BuiltinPropertyGetSetHelper
{
public:
    explicit CustomPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :BuiltinPropertyGetSetHelper( xModel )
    {
    }
    virtual uno::Any getPropertyValue( const OUString& rPropName ) override
    {
        return getUserDefinedProperties()->getPropertyValue(rPropName);
    }
    virtual void setPropertyValue(
            const OUString& rPropName, const uno::Any& rValue) override
    {
        return getUserDefinedProperties()->setPropertyValue(rPropName, rValue);
    }
};

class StatisticPropertyGetSetHelper : public PropertGetSetHelper
{
    SwDocShell* mpDocShell;
    uno::Reference< beans::XPropertySet > mxModelProps;
public:
    explicit StatisticPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :PropertGetSetHelper( xModel ) , mpDocShell( nullptr )
    {
            mxModelProps.set( m_xModel, uno::UNO_QUERY_THROW );
            mpDocShell = word::getDocShell( xModel );
    }
    virtual uno::Any getPropertyValue( const OUString& rPropName ) override
    {
        try
        {
            // Characters, ParagraphCount & WordCount are available from
            // the model ( and additionally these also update the statics object )
            return mxModelProps->getPropertyValue( rPropName );
        }
        catch (const uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("sw.vba", "Got exception " << exceptionToString(ex));
        }
        uno::Any aReturn;
        if ( rPropName == "LineCount" ) // special processing needed
        {
            if ( mpDocShell )
            {
                SwFEShell* pFEShell = mpDocShell->GetFEShell();
                if(pFEShell)
                {
                    aReturn <<= pFEShell->GetLineCount();
                }
            }
        }
        else
        {
            uno::Sequence< beans::NamedValue > const stats(
                m_xDocProps->getDocumentStatistics());

            sal_Int32 nLen = stats.getLength();
            bool bFound = false;
            for ( sal_Int32 index = 0; index < nLen && !bFound ; ++index )
            {
                if ( rPropName == stats[ index ].Name )
                {
                    aReturn = stats[ index ].Value;
                    bFound = true;
                }
            }
            if ( !bFound )
                throw uno::RuntimeException(); // bad Property
        }
        return aReturn;
    }

    virtual void setPropertyValue( const OUString& rPropName, const uno::Any& aValue ) override
    {
        uno::Sequence< beans::NamedValue > stats(
                m_xDocProps->getDocumentStatistics());

        sal_Int32 nLen = stats.getLength();
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            if ( rPropName == stats[ index ].Name )
            {
                stats[ index ].Value = aValue;
                m_xDocProps->setDocumentStatistics(stats);
                break;
            }
        }
    }
};

class DocPropInfo
{
public:
    OUString msMSODesc;
    OUString msOOOPropName;
    std::shared_ptr< PropertGetSetHelper > mpPropGetSetHelper;

    static DocPropInfo createDocPropInfo( const OUString& sDesc, const OUString& sPropName, std::shared_ptr< PropertGetSetHelper > const & rHelper )
    {
        DocPropInfo aItem;
        aItem.msMSODesc = sDesc;
        aItem.msOOOPropName = sPropName;
        aItem.mpPropGetSetHelper = rHelper;
        return aItem;
    }

    static DocPropInfo createDocPropInfo( const sal_Char* sDesc, const sal_Char* sPropName, std::shared_ptr< PropertGetSetHelper > const & rHelper )
    {
        return createDocPropInfo( OUString::createFromAscii( sDesc ), OUString::createFromAscii( sPropName ), rHelper );
    }
    uno::Any getValue()
    {
        if ( mpPropGetSetHelper.get() )
            return mpPropGetSetHelper->getPropertyValue( msOOOPropName );
        return uno::Any();
    }
    void setValue( const uno::Any& rValue )
    {
        if ( mpPropGetSetHelper.get() )
            mpPropGetSetHelper->setPropertyValue( msOOOPropName, rValue );
    }
    uno::Reference< beans::XPropertySet > getUserDefinedProperties()
    {
        uno::Reference< beans::XPropertySet > xProps;
        if ( mpPropGetSetHelper.get() )
            return mpPropGetSetHelper->getUserDefinedProperties();
        return xProps;
    }
};

typedef std::unordered_map< sal_Int32, DocPropInfo > MSOIndexToOODocPropInfo;

class BuiltInIndexHelper
{
    MSOIndexToOODocPropInfo m_docPropInfoMap;

public:
    explicit BuiltInIndexHelper( const uno::Reference< frame::XModel >& xModel )
    {
        std::shared_ptr< PropertGetSetHelper > aStandardHelper( new BuiltinPropertyGetSetHelper( xModel ) );
        std::shared_ptr< PropertGetSetHelper > aUsingStatsHelper( new StatisticPropertyGetSetHelper( xModel ) );

        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyTitle ] = DocPropInfo::createDocPropInfo( "Title", "Title", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertySubject ] = DocPropInfo::createDocPropInfo( "Subject", "Subject", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyAuthor ] = DocPropInfo::createDocPropInfo( "Author", "Author", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyKeywords ] = DocPropInfo::createDocPropInfo( "Keywords", "Keywords", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyComments ] = DocPropInfo::createDocPropInfo( "Comments", "Description", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyTemplate ] = DocPropInfo::createDocPropInfo( "Template", "Template", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyLastAuthor ] = DocPropInfo::createDocPropInfo( "Last author", "ModifiedBy", aStandardHelper ); // doesn't seem to exist - throw or return nothing ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyRevision ] = DocPropInfo::createDocPropInfo( "Revision number", "EditingCycles", aStandardHelper ); // doesn't seem to exist - throw or return nothing ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyAppName ] = DocPropInfo::createDocPropInfo( "Application name", "Generator", aStandardHelper ); // doesn't seem to exist - throw or return nothing ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyTimeLastPrinted ] = DocPropInfo::createDocPropInfo( "Last print date", "PrintDate", aStandardHelper ); // doesn't seem to exist - throw or return nothing ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyTimeCreated ] = DocPropInfo::createDocPropInfo( "Creation date", "CreationDate", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyTimeLastSaved ] = DocPropInfo::createDocPropInfo( "Last save time", "ModifyDate", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyVBATotalEdit ] = DocPropInfo::createDocPropInfo( "Total editing time", "EditingDuration", aStandardHelper ); // Not sure if this is correct
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyPages ] = DocPropInfo::createDocPropInfo( "Number of pages", "PageCount", aUsingStatsHelper ); // special handling required ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyWords ] = DocPropInfo::createDocPropInfo( "Number of words", "WordCount", aUsingStatsHelper ); // special handling require ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyCharacters ] = DocPropInfo::createDocPropInfo( "Number of characters", "CharacterCount", aUsingStatsHelper ); // special handling required ?
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertySecurity ] = DocPropInfo::createDocPropInfo( "Security", "", aStandardHelper ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyCategory ] = DocPropInfo::createDocPropInfo( "Category", "Category", aStandardHelper ); // hacked in
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyFormat ] = DocPropInfo::createDocPropInfo( "Format", "", aStandardHelper ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyManager ] = DocPropInfo::createDocPropInfo( "Manager", "Manager", aStandardHelper ); // hacked in
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyCompany ] = DocPropInfo::createDocPropInfo( "Company", "Company", aStandardHelper ); // hacked in
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyBytes ] = DocPropInfo::createDocPropInfo( "Number of bytes", "", aStandardHelper ); // doesn't seem to exist - size on disk exists ( for an already saved document ) perhaps it will do ( or we need something else )
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyLines ] = DocPropInfo::createDocPropInfo( "Number of lines", "LineCount", aUsingStatsHelper ); // special handling
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyParas ] = DocPropInfo::createDocPropInfo( "Number of paragraphs", "ParagraphCount", aUsingStatsHelper ); // special handling
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertySlides ] = DocPropInfo::createDocPropInfo( "Number of slides", "" , aStandardHelper ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyNotes ] = DocPropInfo::createDocPropInfo( "Number of notes", "", aStandardHelper ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyHiddenSlides ] = DocPropInfo::createDocPropInfo("Number of hidden Slides", "", aStandardHelper  ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyMMClips ] = DocPropInfo::createDocPropInfo( "Number of multimedia clips", "", aStandardHelper ); // doesn't seem to exist
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyHyperlinkBase ] = DocPropInfo::createDocPropInfo( "Hyperlink base", "AutoloadURL", aStandardHelper );
        m_docPropInfoMap[ word::WdBuiltInProperty::wdPropertyCharsWSpaces ] = DocPropInfo::createDocPropInfo( "Number of characters (with spaces)", "", aStandardHelper ); // doesn't seem to be supported
    }

    MSOIndexToOODocPropInfo& getDocPropInfoMap() { return m_docPropInfoMap; }
};

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::XDocumentProperty > SwVbaDocumentProperty_BASE;

class SwVbaBuiltInDocumentProperty : public SwVbaDocumentProperty_BASE
{
protected:
    DocPropInfo mPropInfo;
public:
    SwVbaBuiltInDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo );
    // XDocumentProperty
    virtual void SAL_CALL Delete(  ) override;
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& Name ) override;
    virtual ::sal_Int8 SAL_CALL getType(  ) override;
    virtual void SAL_CALL setType( ::sal_Int8 Type ) override;
    virtual sal_Bool SAL_CALL getLinkToContent(  ) override;
    virtual void SAL_CALL setLinkToContent( sal_Bool LinkToContent ) override;
    virtual uno::Any SAL_CALL getValue(  ) override;
    virtual void SAL_CALL setValue( const uno::Any& Value ) override;
    virtual OUString SAL_CALL getLinkSource(  ) override;
    virtual void SAL_CALL setLinkSource( const OUString& LinkSource ) override;
    //XDefaultProperty
    virtual OUString SAL_CALL getDefaultPropertyName(  ) override { return OUString("Value"); }
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual uno::Sequence<OUString> getServiceNames() override;
};

class SwVbaCustomDocumentProperty : public SwVbaBuiltInDocumentProperty
{
public:

    SwVbaCustomDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo );

    virtual sal_Bool SAL_CALL getLinkToContent(  ) override;
    virtual void SAL_CALL setLinkToContent( sal_Bool LinkToContent ) override;

    virtual OUString SAL_CALL getLinkSource(  ) override;
    virtual void SAL_CALL setLinkSource( const OUString& LinkSource ) override;
    virtual void SAL_CALL Delete(  ) override;
    virtual void SAL_CALL setName( const OUString& Name ) override;
    virtual void SAL_CALL setType( ::sal_Int8 Type ) override;

};

SwVbaCustomDocumentProperty::SwVbaCustomDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo ) : SwVbaBuiltInDocumentProperty( xParent, xContext, rInfo )
{
}

sal_Bool
SwVbaCustomDocumentProperty::getLinkToContent(  )
{
    // #FIXME we need to store the link content somewhere
    return false;
}

void
SwVbaCustomDocumentProperty::setLinkToContent( sal_Bool /*bLinkContent*/ )
{
}

OUString
SwVbaCustomDocumentProperty::getLinkSource(  )
{
    // #FIXME we need to store the link content somewhere
    return OUString();
}

void
SwVbaCustomDocumentProperty::setLinkSource( const OUString& /*rsLinkContent*/ )
{
    // #FIXME we need to store the link source somewhere
}

void SAL_CALL
SwVbaCustomDocumentProperty::setName( const OUString& /*Name*/ )
{
    // setName on existing property ?
    // #FIXME
    // do we need to delete existing property and create a new one?
}

void SAL_CALL
SwVbaCustomDocumentProperty::setType( ::sal_Int8 /*Type*/ )
{
    // setType, do we need to do a conversion?
    // #FIXME the underlying value needs to be changed to the new type
}

void SAL_CALL
SwVbaCustomDocumentProperty::Delete(  )
{
    uno::Reference< beans::XPropertyContainer > xContainer(
            mPropInfo.getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    xContainer->removeProperty( getName() );
}

SwVbaBuiltInDocumentProperty::SwVbaBuiltInDocumentProperty( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo ) : SwVbaDocumentProperty_BASE( xParent, xContext ), mPropInfo( rInfo )
{
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::Delete(  )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

OUString SAL_CALL
SwVbaBuiltInDocumentProperty::getName(  )
{
    return mPropInfo.msMSODesc;
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setName( const OUString& )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

::sal_Int8 SAL_CALL
SwVbaBuiltInDocumentProperty::getType(  )
{
    return lcl_toMSOPropType( getValue().getValueType() );
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setType( ::sal_Int8 /*Type*/ )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

sal_Bool SAL_CALL
SwVbaBuiltInDocumentProperty::getLinkToContent(  )
{
    return false; // built-in always false
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setLinkToContent( sal_Bool /*LinkToContent*/ )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
SwVbaBuiltInDocumentProperty::getValue(  )
{
    uno::Any aRet = mPropInfo.getValue();
    if ( !aRet.hasValue() )
        throw uno::RuntimeException();
    return aRet;
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setValue( const uno::Any& Value )
{
    mPropInfo.setValue( Value );
}

OUString SAL_CALL
SwVbaBuiltInDocumentProperty::getLinkSource(  )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setLinkSource( const OUString& /*LinkSource*/ )
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

OUString
SwVbaBuiltInDocumentProperty::getServiceImplName()
{
    return OUString("SwVbaBuiltinDocumentProperty");
}

uno::Sequence<OUString>
SwVbaBuiltInDocumentProperty::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.DocumentProperty"
    };
    return aServiceNames;
}
typedef ::cppu::WeakImplHelper< css::container::XIndexAccess
        ,css::container::XNameAccess
        ,css::container::XEnumerationAccess
        > PropertiesImpl_BASE;

typedef std::unordered_map< sal_Int32, uno::Reference< XDocumentProperty > > DocProps;

class DocPropEnumeration : public ::cppu::WeakImplHelper< css::container::XEnumeration >
{
    DocProps mDocProps;
    DocProps::iterator mIt;
public:

    explicit DocPropEnumeration( const DocProps& rProps ) : mDocProps( rProps ), mIt( mDocProps.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return mIt != mDocProps.end();
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return uno::makeAny( mIt++->second );
    }
};

typedef std::unordered_map< OUString, uno::Reference< XDocumentProperty > > DocPropsByName;

class BuiltInPropertiesImpl : public PropertiesImpl_BASE
{
protected:

    uno::Reference< frame::XModel > m_xModel;

    DocProps mDocProps;
    DocPropsByName mNamedDocProps;

    public:
    BuiltInPropertiesImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : m_xModel( xModel )
    {
        BuiltInIndexHelper builtIns( m_xModel );
        for ( sal_Int32 index = word::WdBuiltInProperty::wdPropertyTitle; index <= word::WdBuiltInProperty::wdPropertyCharsWSpaces; ++index )
        {
            mDocProps[ index ] = new SwVbaBuiltInDocumentProperty( xParent, xContext, builtIns.getDocPropInfoMap()[ index ] );
            mNamedDocProps[ mDocProps[ index ]->getName() ] = mDocProps[ index ];
        }
    }
// XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return mDocProps.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        // correct the correct by the base class for 1 based indices
        DocProps::iterator it = mDocProps.find( ++Index );
        if ( it == mDocProps.end() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( it->second  );
    }
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        DocPropsByName::iterator it = mNamedDocProps.find( aName );
        return uno::Any( it->second );

    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< OUString > aNames( getCount() );
        OUString* pName = aNames.getArray();
        for (const auto& rEntry : mNamedDocProps)
        {
           *pName = rEntry.first;
           ++pName;
        }
        return aNames;
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        DocPropsByName::iterator it = mNamedDocProps.find( aName );
        if ( it == mNamedDocProps.end() )
            return false;
        return true;
    }
// XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return  cppu::UnoType<XDocumentProperty>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return !mDocProps.empty();
    }
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new DocPropEnumeration( mDocProps );
    }
};

SwVbaBuiltinDocumentProperties::SwVbaBuiltinDocumentProperties( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaDocumentproperties_BASE( xParent, xContext,  uno::Reference< container::XIndexAccess >( new BuiltInPropertiesImpl( xParent, xContext, xModel ) ) )
{
}

uno::Reference< XDocumentProperty > SAL_CALL
SwVbaBuiltinDocumentProperties::Add( const OUString& /*Name*/, sal_Bool /*LinkToContent*/, ::sal_Int8 /*Type*/, const uno::Any& /*value*/, const uno::Any& /*LinkSource*/ )
{
    throw uno::RuntimeException( "not supported for Builtin properties" );
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaBuiltinDocumentProperties::getElementType()
{
    return  cppu::UnoType<XDocumentProperty>::get();
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaBuiltinDocumentProperties::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumAccess->createEnumeration();
}

// ScVbaCollectionBaseImpl
uno::Any
SwVbaBuiltinDocumentProperties::createCollectionObject( const uno::Any& aSource )
{
    // pass through
    return aSource;
}

// XHelperInterface
OUString
SwVbaBuiltinDocumentProperties::getServiceImplName()
{
    return OUString("SwVbaBuiltinDocumentProperties");
}

uno::Sequence<OUString>
SwVbaBuiltinDocumentProperties::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.DocumentProperties"
    };
    return aServiceNames;
}

class CustomPropertiesImpl : public PropertiesImpl_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< beans::XPropertySet > mxUserDefinedProp;
    std::shared_ptr< PropertGetSetHelper > mpPropGetSetHelper;
public:
    CustomPropertiesImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : m_xParent( xParent ), m_xContext( xContext ), m_xModel( xModel )
    {
        // suck in the document( custom ) properties
        mpPropGetSetHelper.reset( new CustomPropertyGetSetHelper( m_xModel ) );
        mxUserDefinedProp.set(mpPropGetSetHelper->getUserDefinedProperties(),
                uno::UNO_SET_THROW);
    };
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return mxUserDefinedProp->getPropertySetInfo()->getProperties().getLength();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        uno::Sequence< beans::Property > aProps = mxUserDefinedProp->getPropertySetInfo()->getProperties();
        if ( Index >= aProps.getLength() )
            throw lang::IndexOutOfBoundsException();
        // How to determine type e.g Date? ( com.sun.star.util.DateTime )
        DocPropInfo aPropInfo = DocPropInfo::createDocPropInfo( aProps[ Index ].Name, aProps[ Index ].Name, mpPropGetSetHelper );
        return uno::makeAny( uno::Reference< XDocumentProperty >( new SwVbaCustomDocumentProperty( m_xParent, m_xContext, aPropInfo ) ) );
    }

    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();

        DocPropInfo aPropInfo = DocPropInfo::createDocPropInfo( aName, aName, mpPropGetSetHelper );
        return uno::makeAny( uno::Reference< XDocumentProperty >( new SwVbaCustomDocumentProperty( m_xParent, m_xContext, aPropInfo ) ) );
    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< beans::Property > aProps = mxUserDefinedProp->getPropertySetInfo()->getProperties();
        uno::Sequence< OUString > aNames( aProps.getLength() );
        OUString* pString = aNames.getArray();
        OUString* pEnd = pString + aNames.getLength();
        beans::Property* pProp = aProps.getArray();
        for ( ; pString != pEnd; ++pString, ++pProp )
            *pString = pProp->Name;
        return aNames;
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        SAL_INFO("sw.vba", "hasByName(" << aName << ") returns " << mxUserDefinedProp->getPropertySetInfo()->hasPropertyByName( aName ) );
        return mxUserDefinedProp->getPropertySetInfo()->hasPropertyByName( aName );
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return  cppu::UnoType<XDocumentProperty>::get();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return getCount() > 0;
    }

    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        // create a map of properties ( the key doesn't matter )
        SAL_INFO("sw.vba", "Creating an enumeration");
        sal_Int32 key = 0;
        sal_Int32 nElem =  getCount();
        DocProps simpleDocPropSnapShot;
        for ( ; key < nElem; ++key )
             simpleDocPropSnapShot[ key ].set( getByIndex( key ), uno::UNO_QUERY_THROW );
        SAL_INFO("sw.vba", "After creating the enumeration");
        return  new DocPropEnumeration( simpleDocPropSnapShot );
    }

    void addProp( const OUString& Name, const uno::Any& Value )
    {
        uno::Reference< beans::XPropertyContainer > xContainer( mxUserDefinedProp, uno::UNO_QUERY_THROW );
        // TODO fixme, perform the necessary Type Value conversions
        xContainer->addProperty( Name, sal_Int16(128), Value );
    }

};

SwVbaCustomDocumentProperties::SwVbaCustomDocumentProperties( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaBuiltinDocumentProperties( xParent, xContext, xModel )
{
    // replace the m_xIndexAccess implementation ( we need a virtual init )
    m_xIndexAccess.set( new CustomPropertiesImpl( xParent, xContext, xModel ) );
    m_xNameAccess.set( m_xIndexAccess, uno::UNO_QUERY_THROW );
}

uno::Reference< XDocumentProperty > SAL_CALL
SwVbaCustomDocumentProperties::Add( const OUString& Name, sal_Bool LinkToContent, ::sal_Int8 /*Type*/, const uno::Any& Value, const uno::Any& LinkSource )
{
    CustomPropertiesImpl* pCustomProps = dynamic_cast< CustomPropertiesImpl* > ( m_xIndexAccess.get() );
    uno::Reference< XDocumentProperty > xDocProp;
    if ( pCustomProps )
    {
        OUString sLinkSource;
        pCustomProps->addProp( Name, Value );

        xDocProp.set( m_xNameAccess->getByName( Name ), uno::UNO_QUERY_THROW );
        xDocProp->setLinkToContent( LinkToContent );

        if ( LinkSource >>= sLinkSource )
           xDocProp->setLinkSource( sLinkSource );
    }
    return xDocProp;
}

// XHelperInterface
OUString
SwVbaCustomDocumentProperties::getServiceImplName()
{
    return OUString("SwVbaCustomDocumentProperties");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
