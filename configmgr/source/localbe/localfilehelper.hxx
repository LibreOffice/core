#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#define CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_

#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/file.hxx>


namespace configmgr
{

    namespace localbe
    {
        namespace css = com::sun::star ;
        namespace uno = css::uno ;
        namespace backend = css::configuration::backend ;

        /**
          Validates a file URL

          @param _sFileURL    URL of the file to validate
          @return bool       true if URL valid, false if URL invalid
          */

        bool isValidFileURL (rtl::OUString const& _sFileURL);
        /**
            Ensures Absolute URL
            @param _sFileURL    URL of the file to validate
            @return bool       true if URL is absolute URL,
                               false if URL is not absolute URL
            */

        bool implEnsureAbsoluteURL(rtl::OUString & _sFileURL);
        /**
            Normalize  URL
            @param _sFileURL    URL of the file to validate
            @param aDirItem    Directory Item
            @return RC         error code
            */

        osl::DirectoryItem::RC implNormalizeURL(rtl::OUString & _sFileURL, osl::DirectoryItem& aDirItem);
        /**
            Normalize  URL
            @param _sFileURL    URL of the file to validate
            @param pContext     pointer to context class
            @param bNothrow     param indicating that exception should not be thrown
            @return bool       true if URL is normalized URL
                               false if URL is not normalized URL
            */


        bool normalizeURL(rtl::OUString & _sFileURL,
                          const uno::Reference<uno::XInterface>& pContext,
                          bool bNothrow = false )
            throw (backend::InsufficientAccessRightsException,
                   backend::BackendAccessException);

         /**
          Validates a file URL

          @param _sFileURL    URL of the file to validate
          @param pContext    pointer to context class
          @throws css::configuration::InvalidBootstrapFileException
          */
        void validateFileURL(const rtl::OUString& _sFileURL,
                             const uno::Reference<uno::XInterface>& pContext)
            throw(css::configuration::InvalidBootstrapFileException);
        /**
           Checks if a Directory exist for a given file URL

          @param _sFileURL    URL of the file to validate
          @param pContext    pointer to context class
          @throws backend::BackendSetupException
          */
        void checkIfDirectory(const rtl::OUString& _sFileURL,
                              const uno::Reference<uno::XInterface>& pContext )
            throw (backend::BackendSetupException);
        /**
           Checks if a File exist for a given file URL
          @param pContext    pointer to context class
          @param _sFileURL    URL of the file to validate
          @throws backend::CannotConnectException
          */
        void checkFileExists(const rtl::OUString& _sFileURL,
                             const uno::Reference<uno::XInterface>& pContext)
            throw (backend::CannotConnectException);

        /**
           Convert Component name to Path name
           @param aComponent    Component name
           */
        rtl::OUString componentToPath(const rtl::OUString& aComponent);

        /// character that can be used to delimit a sub-id within a layerid
        const sal_Unicode k_cLayerIdSeparator   = ':';
        /**
           Convert layer id to Path name
           Format of layer id is <component> [:<sub-id>]
           @param aLayerId  layer id
           */
        rtl::OUString layeridToPath(const rtl::OUString& aLayerId);


        /**
           Checks if Arguement is Optional
          @param aArg   Argument
          @return bool  returns true is Argument is Optional, false otherwise
          */
        bool checkOptionalArg(rtl::OUString& aArg);

        /*
            parses sequence of strings from blank separated list
            @param aList    blank separated list
            @param aTarget  sequence of parsed strings
            */
        void fillFromBlankSeparated(const rtl::OUString& aList,
                                    uno::Sequence<rtl::OUString>& aTarget);

    }
}
#endif
