/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddelink.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-23 14:44:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_DDELINK_HXX
#define SC_DDELINK_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _LNKBASE_HXX //autogen
#include <sfx2/lnkbase.hxx>
#endif
#ifndef _SVT_BROADCAST_HXX
#include <svtools/broadcast.hxx>
#endif
#ifndef SC_MATRIX_HXX
#include "scmatrix.hxx"
#endif

class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class SvStream;

class ScDdeLink : public ::sfx2::SvBaseLink, public SvtBroadcaster
{
private:
static BOOL bIsInUpdate;

    ScDocument*     pDoc;

    String          aAppl;          // Verbindungsdaten
    String          aTopic;
    String          aItem;
    BYTE            nMode;          // Zahlformat-Modus

    BOOL            bNeedUpdate;    // wird gesetzt, wenn Update nicht moeglich war

    ScMatrixRef     pResult;        // Ergebnis

public:
    TYPEINFO();

            ScDdeLink( ScDocument* pD,
                        const String& rA, const String& rT, const String& rI,
                        BYTE nM );
            ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr );
            ScDdeLink( ScDocument* pD, const ScDdeLink& rOther );
    virtual ~ScDdeLink();

    void            Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

                                            // von SvBaseLink ueberladen:
    virtual void    DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    void            NewData(SCSIZE nCols, SCSIZE nRows);

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
    BYTE            GetMode() const     { return nMode; }

    void            ResetValue();           // Wert zuruecksetzen
    void            TryUpdate();

    BOOL            NeedsUpdate() const { return bNeedUpdate; }

    static BOOL     IsInUpdate()        { return bIsInUpdate; }
};


#endif

