/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: migrationprogress.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:05:50 $
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

#ifndef DBACCESS_MIGRATIONPROGRESS_HXX
#define DBACCESS_MIGRATIONPROGRESS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= MigrationProgress
    //====================================================================
    class SAL_NO_VTABLE IMigrationProgress
    {
    public:
        virtual void    startObject( const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rCurrentAction, const sal_uInt32 _nRange ) = 0;
        virtual void    setObjectProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue ) = 0;
        virtual void    endObject() = 0;
        virtual void    start( const sal_uInt32 _nOverallRange ) = 0;
        virtual void    setOverallProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue ) = 0;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MIGRATIONPROGRESS_HXX
