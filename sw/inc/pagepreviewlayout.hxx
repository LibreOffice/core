#ifndef _PAGEPREVIEWLAYOUT_HXX
#define _PAGEPREVIEWLAYOUT_HXX

// template class <std::vector>
#include <vector>
// datatypes sal_xyz
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
// classes <Point>, <Size> and <Rectangle>
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
// datatype <SwTwips>
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

class ViewShell;
class OutputDevice;
class SwRootFrm;
class SwPageFrm;
class Fraction;
struct PrevwPage;

// =============================================================================
/** page preview functionality in the writer

    OD 11.12.2002 #103492# - class <SwPagePreviewLayout>

    @author OD
*/
class SwPagePreviewLayout
{
private:
    friend class ViewShell;

    const SwTwips mnXFree;
    const SwTwips mnYFree;

    ViewShell& mrParentViewShell;
    const SwRootFrm& mrLayoutRootFrm;

    bool        mbLayoutInfoValid;
    bool        mbLayoutSizesValid;
    bool        mbPaintInfoValid;

    Size        maWinSize;
    sal_uInt16  mnCols;
    sal_uInt16  mnRows;
    sal_uInt16  mnPages;
    // OD 19.02.2003 #107369# - new flag for leaving blank left-top-corner
    bool        mbLeaveLeftTopBlank;

    Size        maMaxPageSize;
    Rectangle   maPreviewDocRect;
    SwTwips     mnColWidth;
    SwTwips     mnRowHeight;
    SwTwips     mnPrevwLayoutWidth;
    SwTwips     mnPrevwLayoutHeight;
    bool        mbDoesLayoutColsFitIntoWindow;
    bool        mbDoesLayoutRowsFitIntoWindow;

    sal_uInt16  mnPaintPhyStartPageNum;
    sal_uInt16  mnPaintStartCol;
    sal_uInt16  mnPaintStartRow;
    bool        mbNoPageVisible;
    Point       maPaintStartPageOffset;
    Point       maPaintPreviewDocOffset;
    Point       maAdditionalPaintOffset;
    Rectangle   maPaintedPrevwDocRect;
    sal_uInt16  mnSelectedPageNum;
    const SwPageFrm*  mpSelectedPageFrm;

    std::vector<PrevwPage*> maPrevwPages;

    /** clear internal data about current page preview

        OD 11.12.2002 #103492#

        @author OD
    */
    void _Clear();

    /** helper method to clear preview page layout sizes

        OD 18.12.2002 #103492#

        @author OD
    */
    void _ClearPrevwLayoutSizes();

    /** helper method to clear data in preview page vectors

        OD 13.12.2002 #103492#

        @author OD
    */
    void _ClearPrevwPageData();

    /** calculate page preview layout sizes

        OD 18.12.2002 #103492#

        @author OD
    */
    void _CalcPrevwLayoutSizes();

    /** apply new zoom at given view shell

        OD 11.12.2002 #103492#

        @author OD

        @param _aNewZoom
        input parameter - new zoom percentage
    */
    void _ApplyNewZoomAtViewShell( sal_uInt8 _aNewZoom );

    /** calculate additional paint offset

        OD 12.12.2002 #103492#
        helper method called by <Prepare> in order to calculate an additional
        paint offset to center output in given window size.
        The booleans <mbDoesLayoutRowsFitIntoWindow> and <mbDoesLayoutColsFitIntoWindow>
        are also determined.
        preconditions:
        (1) preview layout is given (number of rows and columns).
        (2) window size is given.
        (3) height of row and width of column are calculated.
        (4) paint offset of start page is calculated.

        @author OD
    */
    void _CalcAdditionalPaintOffset();

    /** calculate painted preview document rectangle

        OD 12.12.2002 #103492#
        helper method called by <Prepare> in order to calculate the rectangle,
        which will be painted for the document arranged by the given preview
        layout.
        preconditions:
        (1) paint offset of document preview is calculated.
        (2) size of document preview is calculated.
        (3) additional paint offset is calculated - see <_CalcAdditionalPaintOffset>.

        @author OD
    */
    void _CalcDocPrevwPaintRect();

    /** determines preview data for a given page and a given preview offset

        OD 13.12.2002 #103492#

        @author OD

        @param _rPage
        input parameter - constant reference to page frame, for which the
        preview data will be calculated.

        @param _rPrevwOffset
        input parameter - constant reference to the offset the given page has
        in the current preview window.
        Note: Offset can be negative.

        @param _opPrevwPage
        output parameter - calculated preview data.

        @return boolean, indicating, if calculation was successful.
    */
    bool _CalcPreviewDataForPage( const SwPageFrm& _rPage,
                                  const Point& _rPrevwOffset,
                                  PrevwPage* _opPrevwPage );

    /** calculate preview pages

        OD 12.12.2002 #103492#
        helper method called by <Prepare> in order to determine which pages
        will be visible in the current preview and calculate the data needed
        to paint these pages. Also the accessible pages with its needed data
        are determined.

        @author OD
    */
    void _CalcPreviewPages();

    /** get preview page by physical page number

        OD 17.12.2002 #103492#

        @author OD

        @param _nPageNum
        input parameter - physical page number of page, for which the preview
        page will be returned.

        @return pointer to preview page of current preview pages. If page doesn't
        belongs to current preview pages, <0> is returned.
    */
    const PrevwPage* _GetPrevwPageByPageNum( const sal_uInt16 _nPageNum ) const;

    /** paint selection mark at page

        OD 17.12.2002 #103492#

        @author OD
    */
    const void _PaintSelectMarkAtPage( const PrevwPage* _aSelectedPrevwPage ) const;

public:
    /** constructor of <SwPagePreviewLayout>

        OD 11.12.2002 #103492#

        @author OD

        @param _rParentViewShell
        input parameter - reference to the view shell the page preview
        layout belongs to. Reference will be hold as member <mrParentViewShell>.
        Adjustments/Changes at this view shell:
        (1) Adjustment of the mapping mode at the output device.
        (2) Change of the zoom at the view options.
        (3) Preparations for paint of the page preview.

        @param _rLayoutRootFrm
        input parameter - constant reference to the root frame of the layout.
        Reference will be hold as member <mrLayoutRootFrm> in order to get
        access to the page frames.
    */
    SwPagePreviewLayout( ViewShell& _rParentViewShell,
                         const SwRootFrm&    _rLayoutRootFrm );

    /** destructor of <SwPagePreviewLayout>

        OD 17.12.2002 #103492#

        @author
    */
    inline ~SwPagePreviewLayout()
    {
        _ClearPrevwPageData();
    }

    /** init page preview layout

        OD 11.12.2002 #103492#
        initialize the page preview settings for a given layout.
        side effects:
        (1) If parameter <_bCalcScale> is true, mapping mode with calculated
        scaling is set at the output device and the zoom at the view options of
        the given view shell is set with the calculated scaling.

        @author OD

        @param _nCols
        input parameter - initial number of page columns in the preview.

        @param _nRows
        input parameter - initial number of page rows in the preview.

        @param _rPxWinSize
        input parameter - window size in which the preview will be displayed and
        for which the scaling will be calculated.

        @param _orMaxPageSize
        output parameter - maximal size in width and height of all pages

        @param _orPreviewDocSize
        output parameter - size of the document in the proposed preview layout
        included the spacing between the pages.

        @param _bCalcScale
        input parameter - control, if method should calculate the needed
        scaling for the proposed preview layout for the given window size
        and sets the scaling at the output device and the view options.

        @return boolean, indicating, if preview layout is successful initialized.
    */
    bool Init( const sal_uInt16 _nCols,
               const sal_uInt16 _nRows,
               const Size&      _rPxWinSize,
               const bool       _bCalcScale
              );

    /** method to adjust page preview layout to document changes

        OD 18.12.2002 #103492#

        @author OD

        @return boolean, indicating, if preview layout is successful initialized.
    */
    bool ReInit();

    /** prepare paint of page preview

        OD 12.12.2002 #103492#
        With the valid preview layout settings - calculated and set by method
        <Init(..)> - the paint of a specific part of the virtual preview
        document is prepared. The corresponding part is given by either
        a start page (parameter <_nProposedStartPageNum>) or a absolute position
        (parameter <_aProposedStartPoint>).
        The accessibility preview will also be updated via a corresponding
        method call.

        @author OD

        @param _nProposedStartPageNum [0..<number of document pages>]
        input parameter - proposed number of page, which should be painted in
        the left-top-corner in the current output device. input parameter
        <_bStartWithPageAtFirstCol> influences, if proposed page is actual
        painted in the left-top-corner.

        @param _nProposedStartPos [(0,0)..<PreviewDocumentSize>]
        input parameter - proposed absolute position in the virtual preview
        document, which should be painted in the left-top-corner in the current
        output device.

        @param _rPxWinSize
        input parameter - pixel size of window the preview will be painted in.

        @param _onStartPageNum
        output parameter - physical number of page, which will be painted in the
        left-top-corner in the current output device.

        @param _onStartPageVirtNum
        output parameter - virtual number of page, which will be painted in the
        left-top-corner in the current output device.

        @param _orDocPreviewPaintRect
        output parameter - rectangle of preview document, which will be painted.

        @param _bStartWithPageAtFirstCol
        input parameter with default value "true" - controls, if start page
        is set to page in first column the proposed start page is located.

        @return boolean, indicating, if prepare of preview paint was successful.
    */
    bool Prepare( const sal_uInt16 _nProposedStartPageNum,
                  const Point      _aProposedStartPos,
                  const Size&      _rPxWinSize,
                  sal_uInt16&      _onStartPageNum,
                  sal_uInt16&      _onStartPageVirtNum,
                  Rectangle&       _orDocPreviewPaintRect,
                  const bool       _bStartWithPageAtFirstCol = true
                );

    /** get selected page number

        OD 13.12.2002 #103492#

        @author OD
    */
    inline sal_uInt16 SelectedPage()
    {
        return mnSelectedPageNum;
    }

    /** set selected page number

        OD 14.01.2003 #103492#

        @author OD
    */
    inline void SetSelectedPage( sal_uInt16 _nSelectedPageNum )
    {
        mnSelectedPageNum = _nSelectedPageNum;
    }

    /** paint prepared preview

        OD 12.12.2002 #103492#

        @author OD

        @param _aOutRect
        input parameter - Twip rectangle of window, which should be painted.

        @return boolean, indicating, if paint of preview was performed
    */
    bool Paint( const Rectangle _aOutRect ) const;

    /** repaint pages on page preview

        OD 18.12.2002 #103492#
        method to invalidate visible pages due to changes in a different
        view shell.

        @author OD
    */
    void Repaint( const Rectangle _aInvalidCoreRect ) const;

    /** paint to mark new selected page

        OD 17.12.2002 #103492#
        Perform paint for current selected page in order to unmark it.
        Set new selected page and perform paint to mark this page.

        @author OD

        @param _nNewSelectedPage
        input parameter - physical number of page, which will be marked as selected.
    */
    void MarkNewSelectedPage( const sal_uInt16 _nSelectedPage );

    /** calculate start position for new scale

        OD 12.12.2002 #103492#
        calculate new start position for a new scale. Calculation bases on the
        current visible part of the document arranged in the given preview layout.
        preconditions:
        (1) new scaling is already set at the given output device.

        @author OD

        @return Point, start position for new scale
    */
    Point GetPreviewStartPosForNewScale( const Fraction& _aNewScale,
                                         const Fraction& _aOldScale,
                                         const Size&     _aNewWinSize ) const;

    /** determines, if page with given page number is visible in preview

        OD 12.12.2002 #103492#

        @author OD

        @param _nPageNum
        input parameter - physical number of page, for which it will be
        determined, if it is visible.

        @return boolean, indicating, if page with given page number is visible
        in preview.
    */
    bool IsPageVisible( const sal_uInt16 _nPageNum ) const;

    /** calculate data to bring new selected page into view.

        OD 12.12.2002 #103492#

        @author OD

        @param _nHoriMove
        input parameter - positive/negative number of columns the current
        selected page have to be moved.

        @param _nVertMove
        input parameter - positive/negative number of rows the current
        selected page have to be moved.

        @param _orNewSelectedPage
        output parameter - number of new selected page

        @param _orNewStartPage
        output parameter - number of new start page

        @param _orNewStartPos
        output parameter - new start position in document preview

        @return boolean - indicating, that move was sucessful.
    */
    bool CalcStartValuesForSelectedPageMove( const sal_Int16  _nHoriMove,
                                             const sal_Int16  _nVertMove,
                                             sal_uInt16&      _orNewSelectedPage,
                                             sal_uInt16&      _orNewStartPage,
                                             Point&           _orNewStartPos ) const;

    /** checks, if given position is inside a shown document page

        OD 17.12.2002 #103492#

        @author OD

        @param _aPrevwPos
        input parameter - position inside the visible preview window.

        @param _orDocPos
        output parameter - corresponding position in the document, if given
        preview position is inside a shown document page, not an empty page.
        If not, it's value is <Point( 0, 0 )>.

        @param _obPosInEmptyPage
        output parameter - indicates, that given preview position lays inside
        an shown empty page.

        @param _onPageNum
        output parameter - corresponding physical number of page, if given
        preview position is inside a shown document page, considers also empty
        pages. If not, it's value is <0>.

        @return boolean - indicating, that given preview position lays inside
        a shown document preview page, not an empty page.
    */
    bool IsPrevwPosInDocPrevwPage( const Point  _aPrevwPos,
                                   Point&       _orDocPos,
                                   bool&        _obPosInEmptyPage,
                                   sal_uInt16&  _onPageNum ) const;

    inline bool DoesPreviewLayoutRowsFitIntoWindow() const
    {
        return mbDoesLayoutRowsFitIntoWindow;
    }

    inline bool DoesPreviewLayoutColsFitIntoWindow() const
    {
        return mbDoesLayoutColsFitIntoWindow;
    }

    inline bool PreviewLayoutValid() const
    {
        return mbLayoutInfoValid && mbLayoutSizesValid && mbPaintInfoValid;
    }

    /** determine preview window page scroll amount

        OD 17.12.2002 #103492#

        @author OD

        @param _nWinPagesToScroll
        input parameter - number of preview window pages the scroll amount has
        to be calculated for. Negative values for preview window page up
        scrolling, positive values for preview window page down scrolling.

        @return scroll amount in SwTwips
    */
    SwTwips GetWinPagesScrollAmount( const sal_Int16 _nWinPagesToScroll ) const;

    /** determine window preview page the page with the given number is on.

        OD 17.01.2003 #103492#

        @author OD

        @param _nPageNum
        input parameter - physical page number of page, for which the preview
        window page number has to be calculated.

        @return number of preview window page the page with the given physical
        page number is on
    */
    sal_uInt16 GetWinPageNumOfPage( sal_uInt16 _nPageNum ) const;

    /** determine row the page with the given number is in

        OD 17.01.2003 #103492#

        @author OD

        @param _nPageNum
        input parameter - physical page number of page, for which the row in
        preview layout has to be calculated.

        @return number of row the page with the given physical page number is in
    */
    sal_uInt16 GetRowOfPage( sal_uInt16 _nPageNum ) const;

    /** determine column the page with the given number is in

        OD 17.01.2003 #103492#

        @author OD

        @param _nPageNum
        input parameter - physical page number of page, for which the column in
        preview layout has to be calculated.

        @return number of column the page with the given physical page number is in
    */
    sal_uInt16 GetColOfPage( sal_uInt16 _nPageNum ) const;

    // OD 18.12.2002 #103492#
    Size GetMaxPageSize() const;

    // OD 18.12.2002 #103492#
    Size GetPrevwDocSize() const;

    /** get size of a preview page by its physical page number

        OD 15.01.2003 #103492#

        @author OD

        @param _nPageNum
        input parameter - pysical page number of preview page, for which the
        page size has to be returned.

        @return an object of class <Size>
    */
    Size GetPrevwPageSizeByPageNum( sal_uInt16 _nPageNum ) const;
};

#endif // _PAGEPREVIEWLAYOUT_HXX
