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



 #ifndef IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED
 #define IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED

 #ifndef _SAL_TYPES_H_
 #include <sal/types.h>
 #endif

 class SwTxtFmtColl;
 class SwCharFmt;
 class SwFmt;
 class SwFrmFmt;
 class String;
 class SwNumRule;
 class SwPageDesc;

 /** Access to the style pool
 */
 class IDocumentStylePoolAccess
 {
 public:

    /** Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        sie noch nicht, dann erzuege sie
        Ist der String-Pointer definiert, dann erfrage nur die
        Beschreibung der Attribute, !! es legt keine Vorlage an !!
    */
    virtual SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) = 0;

    /** return das geforderte automatische  Format - Basis-Klasse !
    */
    virtual SwFmt* GetFmtFromPool( sal_uInt16 nId ) = 0;

    /** returne das geforderte automatische Format
     */
    virtual SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId ) = 0;

    /**
     */
    virtual SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId ) = 0;

    /** returne die geforderte automatische Seiten-Vorlage
     */
    virtual SwPageDesc* GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) = 0;

    /**
     */
    virtual SwNumRule* GetNumRuleFromPool( sal_uInt16 nId ) = 0;

    /** pruefe, ob diese "Auto-Collection" in Dokument schon/noch benutzt wird
     */
    virtual bool IsPoolTxtCollUsed( sal_uInt16 nId ) const = 0;
    virtual bool IsPoolFmtUsed( sal_uInt16 nId ) const = 0;
    virtual bool IsPoolPageDescUsed( sal_uInt16 nId ) const = 0;

 protected:
    virtual ~IDocumentStylePoolAccess() {};
};

#endif // IDOCUMENTSTYLEPOOLACCESS_HXX_INCLUDED

