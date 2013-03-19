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
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <ooo/vba/word/WdBuiltInProperty.hpp>
#include <ooo/vba/office/MsoDocProperties.hpp>
#include <memory>
#include <boost/shared_ptr.hpp>
#include "wordvbahelper.hxx"
#include "fesh.hxx"
#include "docsh.hxx"
using namespace ::ooo::vba;
using namespace css;

static sal_Int8 lcl_toMSOPropType( const uno::Type& aType ) throw ( lang::IllegalArgumentException )
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
    PropertGetSetHelper( const uno::Reference< frame::XModel >& xModel ):m_xModel( xModel )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> const
            xDocPropSupp(m_xModel, uno::UNO_QUERY_THROW);
        m_xDocProps.set(xDocPropSupp->getDocumentProperties(),
                uno::UNO_SET_THROW);
    }
    virtual ~PropertGetSetHelper() {}
    virtual uno::Any getPropertyValue( const rtl::OUString& rPropName ) = 0;
    virtual void setPropertyValue( const rtl::OUString& rPropName, const uno::Any& aValue ) = 0;
    virtual uno::Reference< beans::XPropertySet > getUserDefinedProperties() {
        return uno::Reference<beans::XPropertySet>(
                m_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    }

};

class BuiltinPropertyGetSetHelper : public PropertGetSetHelper
{
public:
    BuiltinPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :PropertGetSetHelper( xModel )
    {
    }
    virtual uno::Any getPropertyValue( const rtl::OUString& rPropName )
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
    virtual void setPropertyValue( const rtl::OUString& rPropName, const uno::Any& aValue )
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
    CustomPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :BuiltinPropertyGetSetHelper( xModel )
    {
    }
    virtual uno::Any getPropertyValue( const rtl::OUString& rPropName )
    {
        return getUserDefinedProperties()->getPropertyValue(rPropName);
    }
    virtual void setPropertyValue(
            const rtl::OUString& rPropName, const uno::Any& rValue)
    {
        return getUserDefinedProperties()->setPropertyValue(rPropName, rValue);
    }
};

class StatisticPropertyGetSetHelper : public PropertGetSetHelper
{
    SwDocShell* mpDocShell;
    uno::Reference< beans::XPropertySet > mxModelProps;
public:
    StatisticPropertyGetSetHelper( const uno::Reference< frame::XModel >& xModel ) :PropertGetSetHelper( xModel ) , mpDocShell( NULL )
    {
            mxModelProps.set( m_xModel, uno::UNO_QUERY_THROW );
            mpDocShell = word::getDocShell( xModel );
    }
    virtual uno::Any getPropertyValue( const rtl::OUString& rPropName )
    {
        try
        {
            // Characters, ParagraphCount & WordCount are available from
            // the model ( and addtionally these also update the statics object )
            return mxModelProps->getPropertyValue( rPropName );
        }
        catch (const uno::Exception&)
        {
            OSL_TRACE("Got exception");
        }
        uno::Any aReturn;
        if ( rPropName == "LineCount" ) // special processing needed
        {
            if ( mpDocShell )
            {
                SwFEShell* pFEShell = mpDocShell->GetFEShell();
                if(pFEShell)
                {
                    aReturn <<= pFEShell->GetLineCount(sal_False);
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
                if ( rPropName.equals( stats[ index ].Name ) )
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

    virtual void setPropertyValue( const rtl::OUString& rPropName, const uno::Any& aValue )
    {
        uno::Sequence< beans::NamedValue > stats(
                m_xDocProps->getDocumentStatistics());

        sal_Int32 nLen = stats.getLength();
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            if ( rPropName.equals( stats[ index ].Name ) )
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
    rtl::OUString msMSODesc;
    rtl::OUString msOOOPropName;
    boost::shared_ptr< PropertGetSetHelper > mpPropGetSetHelper;

    static DocPropInfo createDocPropInfo( const rtl::OUString& sDesc, const rtl::OUString& sPropName, boost::shared_ptr< PropertGetSetHelper >& rHelper )
    {
        DocPropInfo aItem;
        aItem.msMSODesc = sDesc;
        aItem.msOOOPropName = sPropName;
        aItem.mpPropGetSetHelper = rHelper;
        return aItem;
    }

    static DocPropInfo createDocPropInfo( const sal_Char* sDesc, const sal_Char* sPropName, boost::shared_ptr< PropertGetSetHelper >& rHelper )
    {
        return createDocPropInfo( rtl::OUString::createFromAscii( sDesc ), rtl::OUString::createFromAscii( sPropName ), rHelper );
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


typedef boost::unordered_map< sal_Int32, DocPropInfo > MSOIndexToOODocPropInfo;

class BuiltInIndexHelper
{
    MSOIndexToOODocPropInfo m_docPropInfoMap;
    BuiltInIndexHelper();
public:
    BuiltInIndexHelper( const uno::Reference< frame::XModel >& xModel )
    {
        boost::shared_ptr< PropertGetSetHelper > aStandardHelper( new BuiltinPropertyGetSetHelper( xModel ) );
        boost::shared_ptr< PropertGetSetHelper > aUsingStatsHelper( new StatisticPropertyGetSetHelper( xModel ) );

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


typedef InheritedHelperInterfaceImpl1< ooo::vba::XDocumentProperty > SwVbaDocumentProperty_BASE;

class SwVbaBuiltInDocumentProperty : public SwVbaDocumentProperty_BASE
{
protected:
    DocPropInfo mPropInfo;
public:
    SwVbaBuiltInDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo );
    // XDocumentProperty
    virtual void SAL_CALL Delete(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& Name ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setType( ::sal_Int8 Type ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLinkToContent(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setLinkToContent( ::sal_Bool LinkToContent ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual uno::Any SAL_CALL getValue(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setValue( const uno::Any& Value ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getLinkSource(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setLinkSource( const rtl::OUString& LinkSource ) throw (script::BasicErrorException, uno::RuntimeException);
    //XDefaultProperty
    virtual ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (uno::RuntimeException) { return rtl::OUString("Value"); }
    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual uno::Sequence<rtl::OUString> getServiceNames();
};

class SwVbaCustomDocumentProperty : public SwVbaBuiltInDocumentProperty
{
public:

    SwVbaCustomDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo );

    virtual ::sal_Bool SAL_CALL getLinkToContent(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setLinkToContent( ::sal_Bool LinkToContent ) throw (script::BasicErrorException, uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getLinkSource(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setLinkSource( const rtl::OUString& LinkSource ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL Delete(  ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& Name ) throw (script::BasicErrorException, uno::RuntimeException);
    virtual void SAL_CALL setType( ::sal_Int8 Type ) throw (script::BasicErrorException, uno::RuntimeException);

};


SwVbaCustomDocumentProperty::SwVbaCustomDocumentProperty(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo ) : SwVbaBuiltInDocumentProperty( xParent, xContext, rInfo )
{
}

sal_Bool
SwVbaCustomDocumentProperty::getLinkToContent(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #FIXME we need to store the link content somewhere
    return sal_False;
}

void
SwVbaCustomDocumentProperty::setLinkToContent( sal_Bool /*bLinkContent*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
}

rtl::OUString
SwVbaCustomDocumentProperty::getLinkSource(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #FIXME we need to store the link content somewhere
    return rtl::OUString();
}

void
SwVbaCustomDocumentProperty::setLinkSource( const rtl::OUString& /*rsLinkContent*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #FIXME we need to store the link source somewhere
}

void SAL_CALL
SwVbaCustomDocumentProperty::setName( const ::rtl::OUString& /*Name*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // setName on existing property ?
    // #FIXME
    // do we need to delete existing property and create a new one?
}

void SAL_CALL
SwVbaCustomDocumentProperty::setType( ::sal_Int8 /*Type*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // setType, do we need to do a conversion?
    // #FIXME the underlying value needs to be changed to the new type
}

void SAL_CALL
SwVbaCustomDocumentProperty::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< beans::XPropertyContainer > xContainer(
            mPropInfo.getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    xContainer->removeProperty( getName() );
}

SwVbaBuiltInDocumentProperty::SwVbaBuiltInDocumentProperty( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const DocPropInfo& rInfo ) : SwVbaDocumentProperty_BASE( xParent, xContext ), mPropInfo( rInfo )
{
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

::rtl::OUString SAL_CALL
SwVbaBuiltInDocumentProperty::getName(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return mPropInfo.msMSODesc;
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setName( const rtl::OUString& ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

::sal_Int8 SAL_CALL
SwVbaBuiltInDocumentProperty::getType(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return lcl_toMSOPropType( getValue().getValueType() );
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setType( ::sal_Int8 /*Type*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

::sal_Bool SAL_CALL
SwVbaBuiltInDocumentProperty::getLinkToContent(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return sal_False; // built-in always false
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setLinkToContent( ::sal_Bool /*LinkToContent*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
SwVbaBuiltInDocumentProperty::getValue(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aRet = mPropInfo.getValue();
    if ( !aRet.hasValue() )
        throw uno::RuntimeException();
    return aRet;
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setValue( const uno::Any& Value ) throw (script::BasicErrorException, uno::RuntimeException)
{
    mPropInfo.setValue( Value );
}

rtl::OUString SAL_CALL
SwVbaBuiltInDocumentProperty::getLinkSource(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

void SAL_CALL
SwVbaBuiltInDocumentProperty::setLinkSource( const rtl::OUString& /*LinkSource*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // not valid for Builtin
    throw uno::RuntimeException();
}

rtl::OUString
SwVbaBuiltInDocumentProperty::getServiceImplName()
{
    return rtl::OUString("SwVbaBuiltinDocumentProperty");
}

uno::Sequence<rtl::OUString>
SwVbaBuiltInDocumentProperty::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.DocumentProperty" );
    }
    return aServiceNames;
}
typedef ::cppu::WeakImplHelper3< com::sun::star::container::XIndexAccess
        ,com::sun::star::container::XNameAccess
        ,com::sun::star::container::XEnumerationAccess
        > PropertiesImpl_BASE;

typedef boost::unordered_map< sal_Int32, uno::Reference< XDocumentProperty > > DocProps;

typedef ::cppu::WeakImplHelper1< com::sun::star::container::XEnumeration > DocPropEnumeration_BASE;
class DocPropEnumeration : public DocPropEnumeration_BASE
{
    DocProps mDocProps;
    DocProps::iterator mIt;
public:

    DocPropEnumeration( const DocProps& rProps ) : mDocProps( rProps ), mIt( mDocProps.begin() ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return mIt != mDocProps.end();
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return uno::makeAny( mIt++->second );
    }
};

typedef boost::unordered_map< rtl::OUString, uno::Reference< XDocumentProperty >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > DocPropsByName;

class BuiltInPropertiesImpl : public PropertiesImpl_BASE
{
protected:

    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< frame::XModel > m_xModel;

    DocProps mDocProps;
    DocPropsByName mNamedDocProps;

    public:
    BuiltInPropertiesImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : m_xParent( xParent ), m_xContext( xContext ), m_xModel( xModel )
    {
        BuiltInIndexHelper builtIns( m_xModel );
        for ( sal_Int32 index = word::WdBuiltInProperty::wdPropertyTitle; index <= word::WdBuiltInProperty::wdPropertyCharsWSpaces; ++index )
        {
            mDocProps[ index ] = new SwVbaBuiltInDocumentProperty( xParent, xContext, builtIns.getDocPropInfoMap()[ index ] );
            mNamedDocProps[ mDocProps[ index ]->getName() ] = mDocProps[ index ];
        }
    }
// XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mDocProps.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException )
    {
        // correct the correct by the base class for 1 based indices
        DocProps::iterator it = mDocProps.find( ++Index );
        if ( it == mDocProps.end() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( it->second  );
    }
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        DocPropsByName::iterator it = mNamedDocProps.find( aName );
        return uno::Any( it->second );

    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > aNames( getCount() );
        rtl::OUString* pName = aNames.getArray();
        DocPropsByName::iterator it_end = mNamedDocProps.end();
        for(  DocPropsByName::iterator it = mNamedDocProps.begin(); it != it_end; ++it, ++pName )
           *pName = it->first;
        return aNames;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        DocPropsByName::iterator it = mNamedDocProps.find( aName );
        if ( it == mNamedDocProps.end() )
            return sal_False;
        return sal_True;
    }
// XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return  XDocumentProperty::static_type(0);
    }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return mDocProps.size() > 0;
    }
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new DocPropEnumeration( mDocProps );
    }
};

SwVbaBuiltinDocumentProperties::SwVbaBuiltinDocumentProperties( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaDocumentproperties_BASE( xParent, xContext,  uno::Reference< container::XIndexAccess >( new BuiltInPropertiesImpl( xParent, xContext, xModel ) ) ), m_xModel( xModel )
{
}

uno::Reference< XDocumentProperty > SAL_CALL
SwVbaBuiltinDocumentProperties::Add( const ::rtl::OUString& /*Name*/, ::sal_Bool /*LinkToContent*/, ::sal_Int8 /*Type*/, const uno::Any& /*value*/, const uno::Any& /*LinkSource*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    throw uno::RuntimeException(
        rtl::OUString("not supported for Builtin properties"), uno::Reference< uno::XInterface >() );
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaBuiltinDocumentProperties::getElementType() throw (uno::RuntimeException)
{
    return  XDocumentProperty::static_type(0);
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaBuiltinDocumentProperties::createEnumeration() throw (uno::RuntimeException)
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
rtl::OUString
SwVbaBuiltinDocumentProperties::getServiceImplName()
{
    return rtl::OUString("SwVbaBuiltinDocumentProperties");
}

uno::Sequence<rtl::OUString>
SwVbaBuiltinDocumentProperties::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.DocumentProperties" );
    }
    return aServiceNames;
}

class CustomPropertiesImpl : public PropertiesImpl_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< beans::XPropertySet > mxUserDefinedProp;
    boost::shared_ptr< PropertGetSetHelper > mpPropGetSetHelper;
public:
    CustomPropertiesImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : m_xParent( xParent ), m_xContext( xContext ), m_xModel( xModel )
    {
        // suck in the document( custom ) properties
        mpPropGetSetHelper.reset( new CustomPropertyGetSetHelper( m_xModel ) );
        mxUserDefinedProp.set(mpPropGetSetHelper->getUserDefinedProperties(),
                uno::UNO_SET_THROW);
    };
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mxUserDefinedProp->getPropertySetInfo()->getProperties().getLength();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException )
    {
        uno::Sequence< beans::Property > aProps = mxUserDefinedProp->getPropertySetInfo()->getProperties();
        if ( Index >= aProps.getLength() )
            throw lang::IndexOutOfBoundsException();
        // How to determine type e.g Date? ( com.sun.star.util.DateTime )
        DocPropInfo aPropInfo = DocPropInfo::createDocPropInfo( aProps[ Index ].Name, aProps[ Index ].Name, mpPropGetSetHelper );
        return uno::makeAny( uno::Reference< XDocumentProperty >( new SwVbaCustomDocumentProperty( m_xParent, m_xContext, aPropInfo ) ) );
    }

    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();

        DocPropInfo aPropInfo = DocPropInfo::createDocPropInfo( aName, aName, mpPropGetSetHelper );
        return uno::makeAny( uno::Reference< XDocumentProperty >( new SwVbaCustomDocumentProperty( m_xParent, m_xContext, aPropInfo ) ) );
    }

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< beans::Property > aProps = mxUserDefinedProp->getPropertySetInfo()->getProperties();
        uno::Sequence< rtl::OUString > aNames( aProps.getLength() );
        rtl::OUString* pString = aNames.getArray();
        rtl::OUString* pEnd = ( pString + aNames.getLength() );
        beans::Property* pProp = aProps.getArray();
        for ( ; pString != pEnd; ++pString, ++pProp )
            *pString = pProp->Name;
        return aNames;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        OSL_TRACE("hasByName(%s) returns %d", rtl::OUStringToOString( aName, RTL_TEXTENCODING_UTF8 ).getStr(), mxUserDefinedProp->getPropertySetInfo()->hasPropertyByName( aName ) );
        return mxUserDefinedProp->getPropertySetInfo()->hasPropertyByName( aName );
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return  XDocumentProperty::static_type(0);
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return getCount() > 0;
    }

    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        // create a map of properties ( the key doesn't matter )
        OSL_TRACE("Creating an enumeration");
        sal_Int32 key = 0;
        sal_Int32 nElem =  getCount();
        DocProps simpleDocPropSnapShot;
        for ( ; key < nElem; ++key )
             simpleDocPropSnapShot[ key ].set( getByIndex( key ), uno::UNO_QUERY_THROW );
        OSL_TRACE("After creating the enumeration");
        return  new DocPropEnumeration( simpleDocPropSnapShot );
    }

    void addProp( const ::rtl::OUString& Name, ::sal_Int8 /*Type*/, const uno::Any& Value )
    {
        sal_Int16 attributes = 128;
        uno::Reference< beans::XPropertyContainer > xContainer( mxUserDefinedProp, uno::UNO_QUERY_THROW );
        // TODO fixme, perform the necessary Type Value conversions
        xContainer->addProperty( Name, attributes, Value );
    }

};


SwVbaCustomDocumentProperties::SwVbaCustomDocumentProperties( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : SwVbaBuiltinDocumentProperties( xParent, xContext, xModel )
{
    // replace the m_xIndexAccess implementation ( we need a virtual init )
    m_xIndexAccess.set( new CustomPropertiesImpl( xParent, xContext, xModel ) );
    m_xNameAccess.set( m_xIndexAccess, uno::UNO_QUERY_THROW );
}

uno::Reference< XDocumentProperty > SAL_CALL
SwVbaCustomDocumentProperties::Add( const ::rtl::OUString& Name, ::sal_Bool LinkToContent, ::sal_Int8 Type, const uno::Any& Value, const uno::Any& LinkSource ) throw (script::BasicErrorException, uno::RuntimeException)
{
    CustomPropertiesImpl* pCustomProps = dynamic_cast< CustomPropertiesImpl* > ( m_xIndexAccess.get() );
    uno::Reference< XDocumentProperty > xDocProp;
    if ( pCustomProps )
    {
        rtl::OUString sLinkSource;
        pCustomProps->addProp( Name, Type, Value );

        xDocProp.set( m_xNameAccess->getByName( Name ), uno::UNO_QUERY_THROW );
        xDocProp->setLinkToContent( LinkToContent );

        if ( LinkSource >>= sLinkSource )
           xDocProp->setLinkSource( sLinkSource );
    }
    return xDocProp;
}

// XHelperInterface
rtl::OUString
SwVbaCustomDocumentProperties::getServiceImplName()
{
    return rtl::OUString("SwVbaCustomDocumentProperties");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
