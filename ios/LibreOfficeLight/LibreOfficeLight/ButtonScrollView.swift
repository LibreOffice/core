//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit

/// Scrollable list of buttons.
/// Kind of like a tab bar, but doesn't maintain or switch between views, just calls back on click
open class ButtonScrollView: UIScrollView
{
    var buttonList: ButtonList? = nil

    var buttonClickedCallback: ( (Int) -> () )? = nil

    var selectedIndex: Int?
    {
        get {
            return buttonList?.selectedIndex
        }
        set {
            buttonList?.selectedIndex = selectedIndex
        }
    }

    public override init(frame: CGRect)
    {
        super.init(frame: frame)
    }

    public required init?(coder aDecoder: NSCoder)
    {
        super.init(coder: aDecoder)
    }

    public func setButtonLabels(labels: [String])
    {
        if let bl = buttonList
        {
            bl.removeFromSuperview()
        }
        let bl = ButtonList(frame: CGRect(x:0, y:0, width: self.frame.width, height:44),
                                labels: labels,
                                owner: self)
        self.addSubview(bl)
        self.contentSize = bl.frame.size
        self.buttonList = bl
    }
}

/// Horizontally laid out buttons, living within the owned scroll view
open class ButtonList: UIView
{
    let labels: [String]
    let gap: CGFloat = 10.0
    let topGap: CGFloat = 8
    weak var owner: ButtonScrollView? = nil

    var buttonBackground = UIColor(white: 0.6, alpha: 1)
    var selectedButtonBackground = UIColor.white

    var selectedIndex: Int? = 0
    {
        didSet {
            runOnMain {
                self.highlightSelectedIndex()
            }
        }
    }

    public init(frame: CGRect, labels: [String], owner: ButtonScrollView)
    {
        self.labels = labels
        self.owner = owner
        super.init(frame: frame)
        self.backgroundColor = UIColor(white: 0.9, alpha: 1)

        var idx = 0
        for label in labels
        {
            let b = UIButton(type: .custom)
            b.setTitle(label, for: .normal)
            b.backgroundColor = buttonBackground
            b.contentEdgeInsets = UIEdgeInsets(top: 4, left: 4, bottom: 4, right: 4)
            b.layer.cornerRadius = 4
            b.tag = idx
            b.addTarget(self, action: #selector(buttonTapped), for: UIControlEvents.touchUpInside)
            self.addSubview(b)
            idx += 1
        }
        self.layoutSubviews()
    }

    @objc func buttonTapped(sender: UIButton, forEvent event: UIEvent)
    {
        let idx = sender.tag
        owner?.buttonClickedCallback?(idx)
        self.selectedIndex = idx
    }

    public required init?(coder aDecoder: NSCoder)
    {
        fatalError("init(coder:) has not been implemented")
    }

    public var buttons: [UIButton]
    {
        return self.subviews.flatMap({ $0 as? UIButton })
    }

    open override func layoutSubviews()
    {

        var x: CGFloat = gap
        for button in buttons
        {
            button.sizeToFit()
            let s = button.frame.size
            button.frame = CGRect(x: x, y: topGap, width: s.width, height: s.height)
            x = x + (s.width + gap)
        }
        highlightSelectedIndex()
        self.frame = CGRect(x:0, y: 0, width: x, height: self.frame.height)
    }

    open func highlightSelectedIndex()
    {
        for (index, button) in buttons.enumerated()
        {
            if (index == selectedIndex)
            {
                button.backgroundColor = selectedButtonBackground
                button.setTitleColor(.black, for: .normal)
            }
            else
            {
                button.backgroundColor = buttonBackground
                button.setTitleColor(.white, for: .normal)
            }

        }
    }
}
