/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextTables.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:17:16 $
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
#ifndef _XMLSEARCH_QE_CONTEXTTABLES_HXX_
#define _XMLSEARCH_QE_CONTEXTTABLES_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_USTRING_HXX_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif


namespace xmlsearch {

    namespace qe {

        /* forward declaration */

        class ContextTables;


        /******************************************************************************/


        class Tables
        {
        public:

            Tables( ContextTables* p );

            ~Tables();

            void setTables( ContextTables* p );


        private:

            sal_Int32 initialWordsCachedL_,destsCachedL_,linkTypesCachedL_,seqNumbersCachedL_;
            sal_Int32 *initialWordsCached_,*destsCached_,*linkTypesCached_,*seqNumbersCached_;

        }; // end class Tables


        /******************************************************************************/


        class ContextTables
        {
            friend class Tables;

        public:

            ContextTables( const std::vector< sal_Int32 >& offsets,
                           sal_Int32 contextDataL,sal_Int8 *contextData,
                           sal_Int32 linkNamesL,rtl::OUString *linkNames );

            ~ContextTables();

            void setMicroindex( sal_Int32 docNo ) throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 parentContext( sal_Int32 context );

            rtl::OUString linkName( sal_Int32 context );

            sal_Int32 linkCode( const rtl::OUString& linkName_ );

            bool* getIgnoredElementsSet( sal_Int32& len, /*out*/
                                         const sal_Int32 ignoredElementsL,
                                         const rtl::OUString* ignoredElements );

            bool notIgnored( sal_Int32 ctx,
                             sal_Int32 ignoredElementsL,bool* ignoredElements );

            sal_Int32 firstParentWithCode( const sal_Int32 pos,const sal_Int32 linkCode_ );

            sal_Int32 firstParentWithCode2( sal_Int32 pos,const sal_Int32 linkCode_,const sal_Int32 parentCode );

            sal_Int32 firstParentWithCode3( sal_Int32 pos,sal_Int32 linkCode_,sal_Int32 ancestorCode );

            sal_Int32 firstParentWithCode4(sal_Int32 pos, sal_Int32 linkCodesL,sal_Int32* linkCodes );

            sal_Int32 firstParentWithCode5(sal_Int32 pos,sal_Int32 pathCodesL,sal_Int32* pathCodes);

            sal_Int32 firstParentWithCode7( const sal_Int32 pos,const sal_Int32 linkCode_,const sal_Int32 seq );

            bool isGoverning( sal_Int32 context );

            void resetContextSearch();

            sal_Int32 wordContextLin(sal_Int32 wordNumber);

        private:

            sal_Int32   nTextNodes_,initialWordsIndex_,lastDocNo_;
            sal_Int32   initialWordsL_,destsL_,linkTypesL_,seqNumbersL_,markersL_;
            sal_Int32   *initialWords_,*dests_,*linkTypes_,*seqNumbers_,*markers_;

            sal_Int32 contextDataL_;
            sal_Int8  *contextData_;        // no

            sal_Int32     linkNamesL_;
            rtl::OUString *linkNames_;      // no

            std::vector<Tables*>     cache_;
            std::vector< sal_Int32 > kTable_,auxArray_;

            const std::vector< sal_Int32 >& offsets_;

        }; // end class ContextTables


        /******************************************************************************/

    }    // end namespace qe

}      // end namespace xmlsearch



#endif
