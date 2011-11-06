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



#ifndef _SD_CUSSHOW_HXX
#define _SD_CUSSHOW_HXX

#include <tools/list.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

class SdDrawDocument;
class SdPage;

/*************************************************************************
|*
|* CustomShow
|*
\************************************************************************/
class SD_DLLPUBLIC SdCustomShow : public List
{
private:
    String          aName;
    SdDrawDocument* pDoc;

    // this is a weak reference to a possible living api wrapper for this custom show
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoCustomShow;

    // forbidden and not implemented
    SdCustomShow();

public:
    // single argument ctors shall be explicit
    explicit SdCustomShow(SdDrawDocument* pDrawDoc);
    SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow );

    virtual ~SdCustomShow();

    // @@@ copy ctor, but no copy assignment? @@@
    SdCustomShow( const SdCustomShow& rShow );

    void   SetName(const String& rName);
    String GetName() const;

    SdDrawDocument* GetDoc() const { return pDoc; }

    void ReplacePage( const SdPage* pOldPage, const SdPage* pNewPage );
    void RemovePage( const SdPage* pPage );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoCustomShow();
};

#endif      // _SD_CUSSHOW_HXX

