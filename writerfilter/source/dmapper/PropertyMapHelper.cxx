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



#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <resourcemodel/TagLogger.hxx>
#include "PropertyMapHelper.hxx"

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
            pTag->addAttr("exception", "true");
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
