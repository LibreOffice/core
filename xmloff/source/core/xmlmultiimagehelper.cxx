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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <xmloff/xmlmultiimagehelper.hxx>
#include <rtl/ustring.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    sal_uInt32 getQualityIndex(const rtl::OUString& rString)
    {
        sal_uInt32 nRetval(0);

        // pixel formats first
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".bmp")))
        {
            return 10;
        }
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".gif")))
        {
            return 20;
        }
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".jpg")))
        {
            return 30;
        }
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".png")))
        {
            return 40;
        }

        // vector formats, prefer always
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".svm")))
        {
            return 1000;
        }
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".wmf")))
        {
            return 1010;
        }
        if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".emf")))
        {
            return 1020;
        }
        else if(rString.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".svg")))
        {
            return 1030;
        }

        return nRetval;
    }
}

//////////////////////////////////////////////////////////////////////////////

multiImageImportHelper::multiImageImportHelper()
:   maImplContextVector(),
    mbSupportsMultipleContents(false)
{
}

multiImageImportHelper::~multiImageImportHelper()
{
    while(!maImplContextVector.empty())
    {
        delete *(maImplContextVector.end() - 1);
        maImplContextVector.pop_back();
    }
}

const SvXMLImportContext* multiImageImportHelper::solveMultipleImages()
{
    SvXMLImportContext* pRetval = 0;

    if(maImplContextVector.size())
    {
        if(maImplContextVector.size() > 1)
        {
            // multiple child contexts were imported, decide which is the most valuable one
            // and remove the rest
            sal_uInt32 nIndexOfPreferred(maImplContextVector.size());
            sal_uInt32 nBestQuality(0), a(0);

            for(a = 0; a < maImplContextVector.size(); a++)
            {
                const rtl::OUString aStreamURL(getGraphicURLFromImportContext(**maImplContextVector[a]));
                const sal_uInt32 nNewQuality(getQualityIndex(aStreamURL));

                if(nNewQuality > nBestQuality)
                {
                    nBestQuality = nNewQuality;
                    nIndexOfPreferred = a;
                }
            }

            // correct if needed, default is to use the last entry
            if(nIndexOfPreferred >= maImplContextVector.size())
            {
                nIndexOfPreferred = maImplContextVector.size() - 1;
            }

            // get the winner
            pRetval = *maImplContextVector[nIndexOfPreferred];

            // remove the rest from parent
            for(a = 0; a < maImplContextVector.size(); a++)
            {
                if(a != nIndexOfPreferred)
                {
                    SvXMLImportContext& rCandidate = **maImplContextVector[a];

                    if(pRetval)
                    {
                        // #124143# evtl. copy imported GluePoints before deprecating
                        // this graphic and context
                        pRetval->onDemandRescueUsefulDataFromTemporary(rCandidate);
                    }

                    removeGraphicFromImportContext(rCandidate);
                }
            }
        }
        else
        {
            // only one, winner is implicit
            pRetval = *maImplContextVector[0];
        }
    }

    return pRetval;
}

void multiImageImportHelper::addContent(const SvXMLImportContext& rSvXMLImportContext)
{
    if(dynamic_cast< const SvXMLImportContext* >(&rSvXMLImportContext))
    {
        maImplContextVector.push_back(new SvXMLImportContextRef(const_cast< SvXMLImportContext* >(&rSvXMLImportContext)));
    }
}

//////////////////////////////////////////////////////////////////////////////
//eof
