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



 #ifndef IDOCUMENTSTATISTICS_HXX_INCLUDED
 #define IDOCUMENTSTATISTICS_HXX_INCLUDED

 struct SwDocStat;

 /** Document statistics information
 */
 class IDocumentStatistics
 {
 public:

    /** die DocInfo hat siche geaendert (Notify ueber die DocShell)
        stosse die entsp. Felder zum Updaten an.
    */
    virtual void DocInfoChgd() = 0;

    /** Dokument - Statistics
    */
    virtual const SwDocStat &GetDocStat() const = 0;

    /**
    */
    virtual void SetDocStat(const SwDocStat& rStat) = 0;

    /**
    */
    virtual void UpdateDocStat(SwDocStat& rStat) = 0;

protected:
    virtual ~IDocumentStatistics() {};
 };

 #endif // IDOCUMENTSTATISTICS_HXX_INCLUDED
