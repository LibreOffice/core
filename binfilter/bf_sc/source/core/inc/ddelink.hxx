/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DDELINK_HXX
#define SC_DDELINK_HXX

#ifndef _LNKBASE_HXX //autogen
#include <bf_so3/lnkbase.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <bf_svtools/brdcst.hxx>
#endif
namespace binfilter {

class ScDocument;
class ScMatrix;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;

class ScDdeLink : public ::binfilter::SvBaseLink, public SfxBroadcaster
{
private:
static BOOL	bIsInUpdate;

    ScDocument*		pDoc;

    String			aAppl;			// Verbindungsdaten
    String			aTopic;
    String			aItem;
    BYTE			nMode;			// Zahlformat-Modus

    BOOL			bNeedUpdate;	// wird gesetzt, wenn Update nicht moeglich war

    ScMatrix*		pResult;		// Ergebnis

public:
    TYPEINFO();

            ScDdeLink( ScDocument* pD,
                         const String& rA, const String& rT, const String& rI,
                        BYTE nM );
            ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr );
    virtual ~ScDdeLink();

    void			Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

                                            // von SvBaseLink ueberladen:
    virtual void	DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

                                            // von SfxBroadcaster ueberladen:

                                            // fuer Interpreter:

    const ScMatrix*	GetResult() const	{ return pResult; }
    ScMatrix* GetResult()	{return pResult; }

    const String&	GetAppl() const		{ return aAppl; }
    const String&	GetTopic() const	{ return aTopic; }
    const String&	GetItem() const		{ return aItem; }
    BYTE			GetMode() const		{ return nMode; }

    void			TryUpdate();
};


} //namespace binfilter
#endif

