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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "xmlDataSourceSetting.hxx"
#include "xmlDataSource.hxx"
#include <xmloff/xmluconv.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDataSourceSetting)

OXMLDataSourceSetting::OXMLDataSourceSetting( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLDataSourceSetting* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
    ,m_bIsList(sal_False)
{
    DBG_CTOR(OXMLDataSourceSetting,NULL);

    m_aPropType = ::getVoidCppuType();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceInfoElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DATA_SOURCE_SETTING_IS_LIST:
                m_bIsList = sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("true"));
                break;
            case XML_TOK_DATA_SOURCE_SETTING_TYPE:
                {
                    // needs to be translated into a ::com::sun::star::uno::Type
                    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Type, MapString2Type );
                    static MapString2Type s_aTypeNameMap;
                    if (!s_aTypeNameMap.size())
                    {
                        s_aTypeNameMap[GetXMLToken( XML_BOOLEAN)]   = ::getBooleanCppuType();
                        s_aTypeNameMap[GetXMLToken( XML_FLOAT)]     = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_DOUBLE)]    = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_STRING)]    = ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_INT)]       = ::getCppuType( static_cast< sal_Int32* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_SHORT)]     = ::getCppuType( static_cast< sal_Int16* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_VOID)]      = ::getVoidCppuType();
                    }

                    const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find(sValue);
                    OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OXMLDataSourceSetting::OXMLDataSourceSetting: invalid type!");
                    if (s_aTypeNameMap.end() != aTypePos)
                        m_aPropType = aTypePos->second;
                }
                break;
            case XML_TOK_DATA_SOURCE_SETTING_NAME:
                m_aSetting.Name = sValue;
                break;
        }
    }

}
// -----------------------------------------------------------------------------

OXMLDataSourceSetting::~OXMLDataSourceSetting()
{
    DBG_DTOR(OXMLDataSourceSetting,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLDataSourceSetting::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDataSourceInfoElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DATA_SOURCE_SETTING:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), nPrefix, rLocalName,xAttrList);
            break;
        case XML_TOK_DATA_SOURCE_SETTING_VALUE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), nPrefix, rLocalName,xAttrList,this );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLDataSourceSetting::EndElement()
{
    if ( m_aSetting.Name.getLength() )
    {
        if ( m_bIsList && m_aInfoSequence.getLength() )
            m_aSetting.Value <<= m_aInfoSequence;

        // if our property is of type string, but was empty, ensure that
        // we don't add a VOID value
        if ( !m_bIsList && ( m_aPropType.getTypeClass() == TypeClass_STRING ) && !m_aSetting.Value.hasValue() )
            m_aSetting.Value <<= ::rtl::OUString();

        GetOwnImport().addInfo(m_aSetting);
    }
}
// -----------------------------------------------------------------------------
void OXMLDataSourceSetting::Characters( const ::rtl::OUString& rChars )
{
    if ( m_pContainer )
        m_pContainer->addValue(rChars);
}
// -----------------------------------------------------------------------------
void OXMLDataSourceSetting::addValue(const ::rtl::OUString& _sValue)
{
    Any aValue;
    if( TypeClass_VOID != m_aPropType.getTypeClass() )
        aValue = convertString(m_aPropType, _sValue);

    if ( !m_bIsList )
        m_aSetting.Value = aValue;
    else
    {
        sal_Int32 nPos = m_aInfoSequence.getLength();
        m_aInfoSequence.realloc(nPos+1);
        m_aInfoSequence[nPos] = aValue;
    }
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLDataSourceSetting::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
Any OXMLDataSourceSetting::convertString(const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters)
{
    ODBFilter& rImporter = GetOwnImport();
    Any aReturn;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue;
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
            rImporter.GetMM100UnitConverter().convertBool(bValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OString("OXMLDataSourceSetting::convertString: could not convert \"")
                +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\" into a boolean!"));
            aReturn <<= bValue;
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
        #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
        #endif
                rImporter.GetMM100UnitConverter().convertNumber(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("OXMLDataSourceSetting::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into an integer!"));
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= (sal_Int16)nValue;
                else
                    aReturn <<= (sal_Int32)nValue;
                break;
            }
        case TypeClass_HYPER:
        {
            OSL_FAIL("OXMLDataSourceSetting::convertString: 64-bit integers not implemented yet!");
        }
        break;
        case TypeClass_DOUBLE:
        {
            double nValue = 0.0;
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
            rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OString("OXMLDataSourceSetting::convertString: could not convert \"")
                +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\" into a double!"));
            aReturn <<= (double)nValue;
        }
        break;
        case TypeClass_STRING:
            aReturn <<= _rReadCharacters;
            break;
        default:
            OSL_FAIL("OXMLDataSourceSetting::convertString: invalid type class!");
    }

    return aReturn;
}

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
