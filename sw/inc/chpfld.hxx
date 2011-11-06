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


#ifndef SW_CHPFLD_HXX
#define SW_CHPFLD_HXX

#include "fldbas.hxx"


class SwFrm;
class SwCntntNode;
class SwTxtNode;


enum SwChapterFormat
{
    CF_BEGIN,
    CF_NUMBER = CF_BEGIN,       // nur die Kapitelnummer
    CF_TITLE,                   // nur die "Ueberschrift"
    CF_NUM_TITLE,               // Kapitelnummer und "Ueberschrift"
    CF_NUMBER_NOPREPST,         // nur die Kapitelnummer ohne Post/Prefix
    CF_NUM_NOPREPST_TITLE,      // Kapitelnummer ohne Post/Prefix und "Ueberschrift"
    CF_END
};

/*--------------------------------------------------------------------
    Beschreibung: Kapitel
 --------------------------------------------------------------------*/

class SwChapterFieldType : public SwFieldType
{
public:
    SwChapterFieldType();

    virtual SwFieldType*    Copy() const;

};



/*--------------------------------------------------------------------
    Beschreibung: Kapitelnummer
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwChapterField : public SwField
{
    friend class SwChapterFieldType;
    sal_uInt8 nLevel;
    String sTitle, sNumber, sPre, sPost;

    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    SwChapterField(SwChapterFieldType*, sal_uInt32 nFmt = 0);

    // --> OD 2008-02-14 #i53420#
//    void ChangeExpansion( const SwFrm*,
//                          const SwTxtNode*,
//                          sal_Bool bSrchNum = sal_False);
    void ChangeExpansion( const SwFrm*,
                          const SwCntntNode*,
        sal_Bool bSrchNum = sal_False);
    // <--
    void ChangeExpansion(const SwTxtNode &rNd, sal_Bool bSrchNum);

    inline sal_uInt8 GetLevel() const;
    inline void SetLevel(sal_uInt8);

    inline const String& GetNumber() const;
    inline const String& GetTitle() const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_uInt8 SwChapterField::GetLevel() const   { return nLevel; }
inline void SwChapterField::SetLevel(sal_uInt8 nLev) { nLevel = nLev; }
inline const String& SwChapterField::GetNumber() const { return sNumber; }
inline const String& SwChapterField::GetTitle() const { return sTitle; }

#endif // SW_CHPFLD_HXX
