#ifndef INCLUDED_X_NOTE_HELPER_HXX
#define INCLUDED_X_NOTE_HELPER_HXX

#ifndef INLCUED_PLCF_HXX
#include <PLCF.hxx>
#endif

#ifndef INCLUDED_WW8_DOCUMENT_IMPL_HXX
#include <WW8DocumentImpl.hxx>
#endif

namespace doctok
{
/**
   A helper class for footnotes and endnotes and annotations(aka X-notes).
 */
template <class T>
class XNoteHelper
{
    /// Character positions of X-notes in footnote document
    WW8StructBase::Pointer_t mpCps;

    /// Character positions of references to X-notes
    typename PLCF<T>::Pointer_t mpRefs;

    /// piece table for CP to FC conversion
    WW8PieceTable::Pointer_t mpPieceTable;

    /// the document
    WW8DocumentImpl * mpDocument;

    /// type of X-notes handled by this helper (PROP_FOOTNOTE, PROP_ENDNOTE)
    PropertyType meType;

    /// offset in document where X-note subdocument starts
    CpAndFc mCpAndFcOffset;

    /**
       Return CpAndFc in document for X-note.

       @param nPos     index of X-note
    */
    CpAndFc getCpAndFc(sal_uInt32 nPos);

    /**
       Return CpAndFc in document for reference to a certain X-note.

       @param nPos     index of the X-note
     */
    CpAndFc getRefCpAndFc(sal_uInt32 nPos);

public:
    typedef boost::shared_ptr<XNoteHelper> Pointer_t;

    XNoteHelper(WW8StructBase::Pointer_t pCps,
                typename PLCF<T>::Pointer_t pRefs,
                WW8PieceTable::Pointer_t pPieceTable,
                WW8DocumentImpl * pDocument,
                PropertyType eType,
                CpAndFc cpAndFcOffset)
    : mpCps(pCps), mpRefs(pRefs), mpPieceTable(pPieceTable),
      mpDocument(pDocument), meType(eType), mCpAndFcOffset(cpAndFcOffset)
    {
    }

    /**
       Initialize.

       Generates CpAndFcs in document for X-notes.
     */
    void init();

    /**
       Return number of X-notes.
     */
    sal_uInt32 getCount() const;

    /**
       Return index of CpAndFc.

       @param rCpAndFc    the CpAndFc to search

       @return the index of the rCpAndFc found or getCount() otherwise
     */
    sal_uInt32 getIndexOfCpAndFc(const CpAndFc & rCpAndFc);

    /**
        Return subdocument containing X-note.

        @param nIndex  index of X-note.
    */
    doctok::Reference<Stream>::Pointer_t get(sal_uInt32 nIndex);

    /**
       Return subdocument containing X-note.

       @param rCpAndFc     CpAndFc of the X-note's reference
    */
    doctok::Reference<Stream>::Pointer_t get(const CpAndFc & rCpAndFc);

    /**
       Return reference data structure of X-note.

       @param nIndex    index of X-note
    */
    T * getRef(sal_uInt32 nIndex);

    /**
       Return reference data structure of X-note.

       @param rCpAndFc     CpAndFc for the X-note's reference
    */
    T * getRef(const CpAndFc & rCpAndFc);
};

}

#endif // INCLUDED_X_NOTE_HELPER_HXX
