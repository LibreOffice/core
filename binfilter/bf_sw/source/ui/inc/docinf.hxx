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
#ifndef _DOCINF_HXX
#define _DOCINF_HXX

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
class Date; 
class Time; 
class DateTime; 
class SvStream; 

namespace binfilter {


// EXTERN ----------------------------------------------------------------
struct DataTupel;

// ENUM -----------------------------------------------------------------
enum InfoType
{
    INFOTYPE_DATE, INFOTYPE_TIME, INFOTYPE_DATETIME,
    INFOTYPE_MODIFYINFO,
    INFOTYPE_STRING, INFOTYPE_STRINGLIST,
    INFOTYPE_BOOL, INFOTYPE_LONG,
    INFOTYPE_TEMPLATE
};


// CLASS -----------------------------------------------------------------
DECLARE_LIST( DataTupelList, DataTupel* )

// CLASS -----------------------------------------------------------------
class VarDocInfos
{
public:
                        VarDocInfos();
                        ~VarDocInfos();

            void		AddDate( String& rName, Date& rDate );
            void		AddTime( String& rName, Time& rTime );
            void		AddDateTime( String& rName, DateTime& rDateTime );

            void		AddString( String& rName, String& rString );
            void		AddStringList( String& rName, String& rStringList );
            void		AddBool( String& rName, BOOL bBool );
            void		AddLong( String& rName, long lLong );

            USHORT		GetCount();
            InfoType	GetType( USHORT nPos );
    const	String&		GetName( USHORT nPos );

    const	Date&		GetDate( USHORT nPos );
    const	Time&		GetTime( USHORT nPos );
    const	DateTime&	GetDateTime( USHORT nPos );

    const	String&		GetString( USHORT nPos );
    const	String&		GetStringList( USHORT nPos );
            BOOL		GetBool( USHORT nPos );
            long		GetLong( USHORT nPos );

friend      SvStream&    operator<<( SvStream&, VarDocInfos& ); //$ ostream
friend      SvStream&    operator>>( SvStream&, VarDocInfos& ); //$ istream

private:
    DataTupelList*		pInfoList;
};

// CLASS -----------------------------------------------------------------
struct ModifyInfo {
    DateTime	aModifiedDateTime;
    String		aModifiedById;
    String		aModifiedByName;

    friend      SvStream&    operator<<( SvStream&, ModifyInfo& ); //$ ostream
    friend      SvStream&    operator>>( SvStream&, ModifyInfo& ); //$ istream
};

// CLASS -----------------------------------------------------------------
class FixDocInfos
{
public:
                        FixDocInfos();
                        ~FixDocInfos();

            void		SetTitle( String& rTitle );
    const	String&		GetTitle() const;

    void				SetCreated( ModifyInfo& rInfo );
    const	ModifyInfo&	GetCreated() const;

            void		SetLastChanged( ModifyInfo& rInfo );
    const	ModifyInfo&	GetLastChanged() const;

            void		SetLastPrinted( ModifyInfo& rInfo );
    const	ModifyInfo&	GetLastPrinted() const;

            void		SetTotalEditingTime( Time& rTime );
    const	Time&		GetTotalEditingTime() const;

friend      SvStream&    operator<<( SvStream&, FixDocInfos& ); //$ ostream
friend      SvStream&    operator>>( SvStream&, FixDocInfos& ); //$ istream

private:
    String			aTitle;

    ModifyInfo 		aCreated;
    ModifyInfo 		aLastChanged;
    ModifyInfo 		aLastPrinted;
    Time			aTotalEditingTime;
};

// CLASS -----------------------------------------------------------------
struct TemplateInfo {
    String		aTitle;
    ULONG		lTemplateId;

                TemplateInfo() {}
                TemplateInfo( String& rName, ULONG lId ) :
                    aTitle( rName ),
                    lTemplateId( lId ) {}

    friend      SvStream&    operator<<( SvStream&, TemplateInfo& ); //$ ostream
    friend      SvStream&    operator>>( SvStream&, TemplateInfo& ); //$ istream
};

// CLASS -----------------------------------------------------------------
DECLARE_LIST( TemplateInfos, TemplateInfo* )

SvStream&    operator<<( SvStream&, TemplateInfos& ); //$ ostream
SvStream&    operator>>( SvStream&, TemplateInfos& ); //$ istream

} //namespace binfilter
#endif // _DOCINF_HXX
