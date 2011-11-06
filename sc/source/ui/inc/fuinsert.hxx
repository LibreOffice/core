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




#ifndef SC_FUINSERT_HXX
#define SC_FUINSERT_HXX

#include "fupoor.hxx"

class FuInsertGraphic : public FuPoor
{
 public:
    FuInsertGraphic( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertGraphic();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertOLE : public FuPoor
{
 public:
    FuInsertOLE( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertOLE();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertChart : public FuPoor
{
 public:
    FuInsertChart( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertChart();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertMedia : public FuPoor
{
 public:
    FuInsertMedia( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertMedia();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

#endif      // _SD_FUINSERT_HXX

