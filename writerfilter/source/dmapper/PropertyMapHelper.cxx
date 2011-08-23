/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyMap.hxx,v $
 * $Revision: 1.18 $
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

#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <resourcemodel/TagLogger.hxx>
#include "PropertyMapHelper.hxx"

#ifdef DEBUG
namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

XMLTag::Pointer_t lcl_TableColumnSeparatorsToTag(const uno::Any & rTableColumnSeparators)
{
    uno::Sequence<text::TableColumnSeparator> aSeq;
    rTableColumnSeparators >>= aSeq;
    
    XMLTag::Pointer_t pResult(new XMLTag("property.TableColumnSeparators"));
    
    sal_uInt32 nLength = aSeq.getLength();
    for (sal_uInt32 n = 0; n < nLength; ++n)
    {
        XMLTag::Pointer_t pTagSeparator(new XMLTag("separator"));
        
        pTagSeparator->addAttr("position", aSeq[n].Position);
        pTagSeparator->addAttr("visible", aSeq[n].IsVisible);
        
        pResult->addTag(pTagSeparator);
    }
    
    return pResult;
}
    
XMLTag::Pointer_t lcl_PropertyValuesToTag(beans::PropertyValues & rValues)
{
    XMLTag::Pointer_t pResult(new XMLTag("propertyValues"));
    
    beans::PropertyValue * pValues = rValues.getArray();
    
    for (sal_Int32 n = 0; n < rValues.getLength(); ++n)
    {
        XMLTag::Pointer_t pTag(new XMLTag("propertyValue"));
        
        pTag->addAttr("name", pValues[n].Name);
        
        try 
        {
            sal_Int32 aInt = 0;
            pValues[n].Value >>= aInt;
            pTag->addAttr("value", aInt);
        }
        catch (...)
        {
        }
        
        if (pValues[n].Name.equalsAscii("TableColumnSeparators"))
        {
            pTag->addTag(lcl_TableColumnSeparatorsToTag(pValues[n].Value));
        }
        
        pResult->addTag(pTag);                               
    }
    
    return pResult;
}   

XMLTag::Pointer_t lcl_PropertyValueSeqToTag(PropertyValueSeq_t & rPropValSeq)
{
    XMLTag::Pointer_t pResult(new XMLTag("PropertyValueSeq"));
    
    beans::PropertyValues * pValues = rPropValSeq.getArray();
    
    for (sal_Int32 n = 0; n < rPropValSeq.getLength(); ++n)
    {
        XMLTag::Pointer_t pTag(lcl_PropertyValuesToTag(pValues[n]));
        
        pResult->addTag(pTag);
    }
    
    return pResult;
}
    
XMLTag::Pointer_t lcl_PropertyValueSeqSeqToTag(PropertyValueSeqSeq_t rPropValSeqSeq)
{
    XMLTag::Pointer_t pResult(new XMLTag("PropertyValueSeq"));
    
    PropertyValueSeq_t * pValues = rPropValSeqSeq.getArray();
    
    for (sal_Int32 n = 0; n < rPropValSeqSeq.getLength(); ++n)
    {
        XMLTag::Pointer_t pTag(lcl_PropertyValueSeqToTag(pValues[n]));
        
        pResult->addTag(pTag);
    }
    
    return pResult;
}
    
}
}
#endif // DEBUG
