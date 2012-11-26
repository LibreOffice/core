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



#ifndef SC_EDITSRC_HXX
#define SC_EDITSRC_HXX

#include "address.hxx"
#include <editeng/unoedsrc.hxx>
#include <svl/lstner.hxx>

#include <memory>

class ScEditEngineDefaulter;
class SvxEditEngineForwarder;

class ScDocShell;
class ScHeaderFooterContentObj;
class ScCellTextData;
class ScHeaderFooterTextData;
class ScAccessibleTextData;
class SdrObject;


class ScHeaderFooterChangedHint : public SfxHint
{
    sal_uInt16 nPart;

public:
                    ScHeaderFooterChangedHint(sal_uInt16 nP);
                    ~ScHeaderFooterChangedHint();

    sal_uInt16          GetPart() const         { return nPart; }
};


//  all ScSharedHeaderFooterEditSource objects for a single text share the same data

class ScSharedHeaderFooterEditSource : public SvxEditSource
{
private:
    ScHeaderFooterTextData*     pTextData;

protected:
    ScHeaderFooterTextData*     GetTextData() const { return pTextData; }   // for ScHeaderFooterEditSource

public:
                                ScSharedHeaderFooterEditSource( ScHeaderFooterTextData* pData );
    virtual                     ~ScSharedHeaderFooterEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*      GetEditEngine();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();

    virtual void                UpdateData();

};

//  ScHeaderFooterEditSource with local copy of ScHeaderFooterTextData is used by field objects

class ScHeaderFooterEditSource : public ScSharedHeaderFooterEditSource
{
public:
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent, sal_uInt16 nP );
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj& rContent, sal_uInt16 nP );
    virtual                     ~ScHeaderFooterEditSource();

    virtual SvxEditSource*      Clone() const;
};


//  Data (incl. EditEngine) for cell EditSource is now shared in ScCellTextData

class ScSharedCellEditSource : public SvxEditSource
{
private:
    ScCellTextData*         pCellTextData;

protected:
    ScCellTextData*         GetCellTextData() const { return pCellTextData; }   // for ScCellEditSource

public:
                                ScSharedCellEditSource( ScCellTextData* pData );
    virtual                     ~ScSharedCellEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*      GetEditEngine();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();

    virtual void                UpdateData();

    void                        SetDoUpdateData(sal_Bool bValue);
    sal_Bool                    IsDirty() const;
};

//  ScCellEditSource with local copy of ScCellTextData is used by ScCellFieldsObj, ScCellFieldObj

class ScCellEditSource : public ScSharedCellEditSource
{
public:
                                ScCellEditSource( ScDocShell* pDocSh, const ScAddress& rP );
    virtual                     ~ScCellEditSource();

    virtual SvxEditSource*      Clone() const;
};


class ScAnnotationEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    sal_Bool                    bDataValid;

    SdrObject*                  GetCaptionObj();
public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


//  EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource : public SvxEditSource
{
private:
    SvxTextForwarder*   pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual             ~ScSimpleEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

};

class ScAccessibilityEditSource : public SvxEditSource
{
private:
    ::std::auto_ptr < ScAccessibleTextData > mpAccessibleTextData;

public:
                        ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData );
    virtual             ~ScAccessibilityEditSource();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*   GetViewForwarder();
    virtual SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void                UpdateData();
    virtual SfxBroadcaster&     GetBroadcaster() const;
};

#endif

