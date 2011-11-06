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



#include <resources.hxx>
#include <WW8DocumentImpl.hxx>

namespace writerfilter {
namespace doctok
{

void WW8PICF::resolveNoAuto(Properties & rHandler)
{
    WW8Stream::Pointer_t pStream = getDocument()->getDataStream();

    {
        writerfilter::Reference<Properties>::Pointer_t pContent;
        if (getDocument()->isPicData())
        {
            WW8Value::Pointer_t pValue = createValue(get_ffdata());
            rHandler.attribute(NS_rtf::LN_ffdata, *pValue);
        }
        else
        {
            WW8Value::Pointer_t pValue = createValue(get_DffRecord());
            rHandler.attribute(NS_rtf::LN_DffRecord, *pValue);
        }
    }
}

writerfilter::Reference<Properties>::Pointer_t
WW8PICF::get_DffRecord()
{
    writerfilter::Reference<Properties>::Pointer_t
        pRet(new DffBlock(this, get_cbHeader(), getCount() - get_cbHeader(),
                          0));
    return pRet;
}

writerfilter::Reference<Properties>::Pointer_t
WW8PICF::get_ffdata()
{
    writerfilter::Reference<Properties>::Pointer_t
        pRet(new WW8FFDATA(this, get_cbHeader(), getCount() - get_cbHeader()));

    WW8StructBase::Pointer_t pStruct
        (new WW8StructBase(this, get_cbHeader(),
                           getCount() - get_cbHeader()));

    pStruct->dump(output);

    return pRet;
}

writerfilter::Reference<Properties>::Pointer_t
WW8FSPA::get_shape()
{
    return getDocument()->getShape(get_spid());
}

void WW8FFDATA::resolveNoAuto(Properties & rHandler)
{
    WW8DocumentImpl * pDocument = getDocument();

    if (pDocument != NULL)
    {
        WW8FLD::Pointer_t pFLD = pDocument->getCurrentFLD();
        WW8Value::Pointer_t pValue = createValue(pFLD->get_flt());

        rHandler.attribute(NS_rtf::LN_FLT, *pValue);

    }
}

sal_uInt32 WW8FFDATA::get_FLT()
{
    sal_uInt32 nResult = 0;

    WW8DocumentImpl * pDocument = getDocument();

    if (pDocument != NULL)
    {
        WW8FLD::Pointer_t pFLD = pDocument->getCurrentFLD();

        if (pFLD.get() != NULL)
            nResult = pFLD->get_flt();
    }

    return nResult;
}

static sal_uInt32 lcl_FFDATA_default_offset(WW8FFDATA & rRef)
{
    return 0xa + (rRef.getU16(0xa) + 2) * 2;
}

static sal_uInt32 lcl_FFDATA_formatting_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_default_offset(rRef);

    switch (rRef.get_FLT())
    {
    case 71: // forms checkbox
    case 83: // forms listbox
        nResult += 2;
        break;
    default:
        nResult += (rRef.getU16(nResult) + 2) * 2;
        break;
    }

    return nResult;;
}

static sal_uInt32 lcl_FFDATA_help_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_formatting_offset(rRef);

    nResult += (rRef.getU16(nResult) + 2) * 2;

    return nResult;;
}

static sal_uInt32 lcl_FFDATA_tooltip_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_help_offset(rRef);

    nResult += (rRef.getU16(nResult) + 2) * 2;

    return nResult;;
}

rtl::OUString WW8FFDATA::get_default()
{
    rtl::OUString sResult;

    sal_uInt32 nOffset = lcl_FFDATA_default_offset(*this);
    switch (get_FLT())
    {
    case 70:
        sResult = getString(nOffset);

        break;
    default:
        break;
    }

    return sResult;
}

rtl::OUString WW8FFDATA::get_formatting()
{
    return getString(lcl_FFDATA_formatting_offset(*this));
}

rtl::OUString WW8FFDATA::get_help()
{
    return getString(lcl_FFDATA_help_offset(*this));
}

rtl::OUString WW8FFDATA::get_tooltip()
{
    rtl::OUString sResult;

    sResult = getString(lcl_FFDATA_tooltip_offset(*this));

    return sResult;
}

sal_uInt16 WW8FFDATA::get_checked()
{
    sal_uInt16 nResult = 0;

    switch (get_FLT())
    {
    case 71:
        nResult = getU16(lcl_FFDATA_default_offset(*this));
        break;
    default:
        break;
    }

    return nResult;
}
}}
