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

#ifndef _XMLOFF_XMLMULTIIMAGEHELPER_HXX
#define _XMLOFF_XMLMULTIIMAGEHELPER_HXX

#include <sal/types.h>
#include <xmloff/xmlimp.hxx>

//////////////////////////////////////////////////////////////////////////////

class multiImageImportHelper
{
private:
    std::vector< SvXMLImportContextRef* >       maImplContextVector;
    bool                                        mbSupportsMultipleContents;

protected:
    /// helper to get the created xShape instance, needs to be overloaded
    virtual rtl::OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const = 0;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const = 0;

public:
    multiImageImportHelper();
    virtual ~multiImageImportHelper();

    /// solve multiple imported images. The most valuable one is chosen,
    /// see imlementation for evtl. changing weights and/or adding filetypes.
    /// returns the winner of the contest (which can be 0 when no candidates)
    const SvXMLImportContext* solveMultipleImages();

    /// add a content to the remembered image import contexts
    void addContent(const SvXMLImportContext& rSvXMLImportContext);

    /// read/write access to boolean switch
    bool getSupportsMultipleContents() const { return mbSupportsMultipleContents; }
    void setSupportsMultipleContents(bool bNew) { mbSupportsMultipleContents = bNew; }
};

//////////////////////////////////////////////////////////////////////////////

#endif  //  _XMLOFF_XMLMULTIIMAGEHELPER_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
