//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import UIKit

public class DocumentOverlaysView: UIView
{
    var searchSubViews: [UIView] = []
    weak var documentTiledView: DocumentTiledView? = nil

    public init(docTiledView: DocumentTiledView)
    {
        self.documentTiledView = docTiledView
        super.init(frame: docTiledView.frame)

        self.layer.compositingFilter = "multiplyBlendMode"
    }

    required public init?(coder aDecoder: NSCoder)
    {
        fatalError("init(coder:) has not been implemented")
    }

    public func clearSearchResults()
    {
        for v in self.searchSubViews
        {
            v.removeFromSuperview()
        }
        searchSubViews = []
    }

    public func setSearchResults(searchResults: SearchResults)
    {
        clearSearchResults()

        guard let documentTiledView = self.documentTiledView else { return }

        if let srs = searchResults.searchResultSelection
        {
            let allTheRects = srs.flatMap { $0.rectsAsCGRects }
                .flatMap { $0 }
                .map { documentTiledView.twipsToPixels(rect: $0) }

            for rect in allTheRects
            {
                let subView = UIView(frame: rect)
                subView.backgroundColor = UIColor.yellow // TODO
                subView.layer.compositingFilter = "multiplyBlendMode"
                self.addSubview(subView)
                searchSubViews.append(subView)
            }

            if let first = allTheRects.first
            {
                if let scrollView = self.superview?.superview as? UIScrollView
                {
                    scrollView.scrollRectToVisible(first, animated: true)
                }
            }
        }
    }
}
