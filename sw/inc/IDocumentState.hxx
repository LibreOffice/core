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



 #ifndef IDOCUMENTSTATE_HXX_INCLUDED
 #define IDOCUMENTSTATE_HXX_INCLUDED

 #include <tools/solar.h>

 /** Get information about the current document state
 */
 class IDocumentState
 {
 public:
       /** Bei Formataenderungen muss das zu Fuss gerufen werden!
    */
    virtual void SetModified() = 0;

    /**
    */
    virtual void ResetModified() = 0;

    /** Dokumentaenderungen?
    */
    virtual bool IsModified() const = 0;

    /** Zustaende ueber Ladezustand
        frueher DocInfo
    */
    virtual bool IsLoaded() const = 0;

    /**
    */
    virtual bool IsUpdateExpFld() const  = 0;

    /**
    */
    virtual bool IsNewDoc() const = 0;

    /**
    */
    virtual bool IsPageNums() const = 0;

    /**
    */
    virtual void SetPageNums(bool b)    = 0;

    /**
    */
    virtual void SetNewDoc(bool b) = 0;

    /**
    */
    virtual void SetUpdateExpFldStat(bool b) = 0;

    /**
    */
    virtual void SetLoaded(bool b = sal_True) = 0;

 protected:
    virtual ~IDocumentState() {};
 };

 #endif // IDOCUMENTSTATE_HXX_INCLUDED
