#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

namespace fileaccess {


    sal_Bool isChild( const rtl::OUString& srcUnqPath,
                      const rtl::OUString& dstUnqPath )
    {
        static sal_Unicode slash = '/';
        // Simple lexical comparison
        sal_Int32 srcL = srcUnqPath.getLength();
        sal_Int32 dstL = dstUnqPath.getLength();

        return (
            ( srcUnqPath == dstUnqPath )
            ||
            ( ( dstL > srcL )
              &&
              ( srcUnqPath.compareTo( dstUnqPath, srcL ) == 0 )
              &&
              ( dstUnqPath[ srcL ] == slash ) )
        );
    }


    rtl::OUString newName(
        const rtl::OUString& aNewPrefix,
        const rtl::OUString& aOldPrefix,
        const rtl::OUString& old_Name )
    {
        sal_Int32 srcL = aOldPrefix.getLength();

        rtl::OUString new_Name = old_Name.copy( srcL );
        new_Name = ( aNewPrefix + new_Name );
        return new_Name;
    }


    rtl::OUString getTitle( const rtl::OUString& aPath )
    {
        sal_Unicode slash = '/';
        sal_Int32 lastIndex = aPath.lastIndexOf( slash );
        return aPath.copy( lastIndex + 1 );
    }


    rtl::OUString getParentName( const rtl::OUString& aFileName )
    {
        sal_Int32 lastIndex = aFileName.lastIndexOf( sal_Unicode('/') );
        rtl::OUString aParent = aFileName.copy( 0,lastIndex );

        if( aParent[ aParent.getLength()-1] == sal_Unicode(':') && aParent.getLength() == 6 )
            aParent += rtl::OUString::createFromAscii( "/" );

        if( 0 == aParent.compareToAscii( "file://" ) )
            aParent = rtl::OUString::createFromAscii( "file:///" );

        return aParent;
    }


    osl::FileBase::RC osl_File_copy( const rtl::OUString& strPath,
                                     const rtl::OUString& strDestPath,
                                     sal_Bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::copy( strPath,strDestPath );
    }


    osl::FileBase::RC osl_File_move( const rtl::OUString& strPath,
                                     const rtl::OUString& strDestPath,
                                     sal_Bool test )
    {
        if( test )
        {
            osl::DirectoryItem aItem;
            if( osl::DirectoryItem::get( strDestPath,aItem ) != osl::FileBase:: E_NOENT )
                return osl::FileBase::E_EXIST;
        }

        return osl::File::move( strPath,strDestPath );
    }


    oslFileError getResolvedURL(rtl_uString* ustrPath, rtl_uString** pustrResolvedURL)
    {
#ifdef TF_FILEURL
        /* TODO: If file exist and is a link get link target URL */
        rtl_uString_assign( pustrResolvedURL, ustrPath );
#else
        rtl_uString_assign( pustrResolvedURL, ustrPath );
#endif

        return osl_File_E_None;
    }



//----------------------------------------------------------------------------
//  makeAbsolute Path
//----------------------------------------------------------------------------

#ifdef TF_FILEURL

    sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString&    aRelPath, rtl::OUString& aAbsPath )
    {
        sal_Int32   nIndex = 6;

        std::vector< rtl::OUString >    aTokenStack;

        // should no longer happen
        OSL_ASSERT( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "//./" ), 4 ) );

        if ( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "file://" ), 7 ) )
            return sal_False;

        aRelPath.getToken( 0, '/', nIndex );

        while ( nIndex >= 0 )
        {
            rtl::OUString   aToken = aRelPath.getToken( 0, '/', nIndex );

            if ( aToken.compareToAscii( ".." ) == 0 )
                aTokenStack.pop_back();
            else
                aTokenStack.push_back( aToken );
        }


        std::vector< rtl::OUString >::iterator it;
        aAbsPath = rtl::OUString::createFromAscii("file:/");

        for ( it = aTokenStack.begin(); it != aTokenStack.end(); it++ )
        {
            aAbsPath += rtl::OUString::createFromAscii( "/" );
            aAbsPath += *it;
        }

        return sal_True;
    }

#else


    sal_Bool SAL_CALL makeAbsolutePath( const rtl::OUString&    aRelPath, rtl::OUString& aAbsPath )
    {
        sal_Int32   nIndex = 0;

        std::vector< rtl::OUString >    aTokenStack;

        if ( 0 != aRelPath.compareTo( rtl::OUString::createFromAscii( "//./" ), 4 ) )
            return sal_False;

        aRelPath.getToken( 0, '/', nIndex );

        while ( nIndex >= 0 )
        {
            rtl::OUString   aToken = aRelPath.getToken( 0, '/', nIndex );

            if ( aToken.compareToAscii( ".." ) == 0 )
                aTokenStack.pop_back();
            else
                aTokenStack.push_back( aToken );
        }


        std::vector< rtl::OUString >::iterator it;
        aAbsPath = rtl::OUString::createFromAscii("file:");

        for ( it = aTokenStack.begin(); it != aTokenStack.end(); it++ )
        {
            aAbsPath += rtl::OUString::createFromAscii( "/" );
            aAbsPath += *it;
        }

        return sal_True;
    }

#endif


    void throw_handler( sal_Int32 errorCode,sal_Int32 minorCode )
    {
        throw com::sun::star::ucb::CommandAbortedException();
    }


}   // end namespace fileaccess
