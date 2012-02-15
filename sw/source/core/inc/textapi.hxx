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



#include <editeng/unoedsrc.hxx>
#include <editeng/unotext.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoforou.hxx>

class SwDoc;

struct SwTextAPIEditSource_Impl;
class SwTextAPIEditSource : public SvxEditSource
{
    SwTextAPIEditSource_Impl* pImpl;

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();
    explicit            SwTextAPIEditSource( const SwTextAPIEditSource& rSource );

public:
                        SwTextAPIEditSource(SwDoc* pDoc);
    virtual             ~SwTextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
    void                SetString( const String& rText );
    OutlinerParaObject* CreateText();
    String              GetText();
};

class SwTextAPIObject : public SvxUnoText
{
    SwTextAPIEditSource* pSource;
public:
                        SwTextAPIObject( SwTextAPIEditSource* p);
    virtual             ~SwTextAPIObject() throw();
    void                DisposeEditSource() { pSource->Dispose(); }
    OutlinerParaObject* CreateText() { return pSource->CreateText(); }
    void                SetString( const String& rText ) { pSource->SetString( rText ); }
    void                SetText( OutlinerParaObject& rText ) { pSource->SetText( rText ); }
    String              GetText() { return pSource->GetText(); }
};
