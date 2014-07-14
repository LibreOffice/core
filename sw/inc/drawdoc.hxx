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


#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

class SwDoc;
class SwDocShell;

//==================================================================

class SwDrawModel : public FmFormModel
{
    SwDoc* pDoc;
public:
    SwDrawModel( SwDoc* pDoc );
    ~SwDrawModel();

    const SwDoc& GetDoc() const { return *pDoc; }
          SwDoc& GetDoc()       { return *pDoc; }

    virtual SdrPage* AllocPage(bool bMasterPage);

    // fuers "load on demand" von Grafiken im DrawingLayer
    virtual SvStream* GetDocumentStream( SdrDocumentStreamInfo& rInfo ) const;

    // fuers Speicher von Rechtecken als Control-Ersatz fuker Versionen < 5.0
    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

    virtual bool IsWriter() const;

protected:
    // --> OD 2006-03-01 #b6382898#
    // overload of <SdrModel::createUnoModel()> is needed to provide corresponding uno model.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();
    // <--
};


#endif
