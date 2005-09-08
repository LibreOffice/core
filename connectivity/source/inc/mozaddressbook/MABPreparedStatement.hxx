/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MABPreparedStatement.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:29:49 $
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

#ifndef _CONNECTIVITY_MAB_PREPAREDSTATEMENT_HXX_
#define _CONNECTIVITY_MAB_PREPAREDSTATEMENT_HXX_

#ifndef _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_
#include "file/FPreparedStatement.hxx"
#endif

namespace connectivity
{
    namespace mozaddressbook
    {
        class OConnection;
        class OMozabPreparedStatement : public file::OPreparedStatement
        {
        protected:
            virtual file::OResultSet* createResultSet();
            // here we create a SQL analyzer which doesn't support any restrictions
            // these are already done by the server side
            virtual file::OSQLAnalyzer* createAnalyzer();
        public:
            //  DECLARE_CTY_DEFAULTS(file::OStatement);
            OMozabPreparedStatement( file::OConnection* _pConnection ) : file::OPreparedStatement( _pConnection ){}
            DECLARE_SERVICE_INFO();
        };
    }
}

#endif //_CONNECTIVITY_MAB_PREPAREDSTATEMENT_HXX_

