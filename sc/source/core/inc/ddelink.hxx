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



#ifndef SC_DDELINK_HXX
#define SC_DDELINK_HXX

#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include <svl/broadcast.hxx>
#include "scmatrix.hxx"

class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class SvStream;

class ScDdeLink : public ::sfx2::SvBaseLink, public SvtBroadcaster
{
private:
static sal_Bool bIsInUpdate;

    ScDocument*     pDoc;

    String          aAppl;          // Verbindungsdaten
    String          aTopic;
    String          aItem;
    sal_uInt8           nMode;          // Zahlformat-Modus

    sal_Bool            bNeedUpdate;    // wird gesetzt, wenn Update nicht moeglich war

    ScMatrixRef     pResult;        // Ergebnis

public:
            ScDdeLink( ScDocument* pD,
                        const String& rA, const String& rT, const String& rI,
                        sal_uInt8 nM );
            ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr );
            ScDdeLink( ScDocument* pD, const ScDdeLink& rOther );
    virtual ~ScDdeLink();

    void            Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

                                            // von SvBaseLink ueberladen:
    virtual void    DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

                                            // von SvtBroadcaster ueberladen:
    virtual void    ListenersGone();

                                            // fuer Interpreter:

    const ScMatrix* GetResult() const           { return pResult; }
    void            SetResult( ScMatrix* pRes ) { pResult = pRes; }

                                            // XML and Excel import after NewData()
    ScMatrixRef     GetModifiableResult()   { return pResult; }

    const String&   GetAppl() const     { return aAppl; }
    const String&   GetTopic() const    { return aTopic; }
    const String&   GetItem() const     { return aItem; }
    sal_uInt8           GetMode() const     { return nMode; }

    void            ResetValue();           // Wert zuruecksetzen
    void            TryUpdate();

    sal_Bool            NeedsUpdate() const { return bNeedUpdate; }

    static sal_Bool     IsInUpdate()        { return bIsInUpdate; }
};


#endif

