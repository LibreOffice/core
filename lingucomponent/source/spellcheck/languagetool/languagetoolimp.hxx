#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>
#include <lingutil.hxx>
#include <memory>
#include <string_view>
#include <o3tl/lru_map.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

// Magical numbers
#define MAX_CACHE_SIZE 10
#define MAX_SUGGESTIONS_SIZE 10
#define PROOFREADING_ERROR 2
#define CURL_TIMEOUT 10L

enum class HTTP_METHOD
{
    HTTP_GET,
    HTTP_POST
};

class LanguageToolGrammarChecker :
    public cppu::WeakImplHelper
    <
        XProofreader,
        XInitialization,
        XServiceInfo,
        XServiceDisplayName
    >
{
    Sequence< Locale > m_aSuppLocales;
    o3tl::lru_map< OUString, Sequence< SingleProofreadingError > > mCachedResults;
    LanguageToolGrammarChecker(const LanguageToolGrammarChecker &) = delete;
    LanguageToolGrammarChecker & operator = (const LanguageToolGrammarChecker &) = delete;
    void parseProofreadingJSONResponse(ProofreadingResult& rResult, std::string_view aJSONBody);
    const std::string makeHttpRequest(std::string_view aURL, HTTP_METHOD method, const OString& aPostData, long& nStatusCode);

public:
    LanguageToolGrammarChecker();
    virtual ~LanguageToolGrammarChecker() override;

    // XSupportedLocales
    virtual Sequence< Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) override;

    // XProofReader
    virtual sal_Bool SAL_CALL isSpellChecker() override;
    virtual ProofreadingResult SAL_CALL doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& aProperties )
    override;

    virtual void SAL_CALL ignoreRule(
        const OUString& aRuleIdentifier,
        const Locale& aLocale
    )
    override;
    virtual void SAL_CALL resetIgnoreRules()
    override;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};