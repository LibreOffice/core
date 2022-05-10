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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


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
    LanguageToolGrammarChecker(const LanguageToolGrammarChecker &) = delete;
    LanguageToolGrammarChecker & operator = (const LanguageToolGrammarChecker &) = delete;
public:
    LanguageToolGrammarChecker();
    virtual ~LanguageToolGrammarChecker() override;

    // XSupportedLocales
    virtual Sequence< Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) override;

    // XProofReader
    virtual sal_Bool SAL_CALL isSpellChecker() override;
    virtual ::css::linguistic2::ProofreadingResult SAL_CALL doProofreading( const ::rtl::OUString& aDocumentIdentifier, const ::rtl::OUString& aText, const ::css::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePosition, ::sal_Int32 nSuggestedBehindEndOfSentencePosition, const ::css::uno::Sequence< ::css::beans::PropertyValue >& aProperties )
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