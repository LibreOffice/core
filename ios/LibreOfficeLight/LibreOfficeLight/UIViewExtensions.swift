//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import UIKit

public extension UIView
{
    public var height: CGFloat
    {
        get
        {
            return frame.size.height
        }
        set
        {
            frame.size.height = newValue
        }
    }

    public var size: CGSize
    {
        get
        {
            return frame.size
        }
        set
        {
            width = newValue.width
            height = newValue.height
        }
    }

    public var width: CGFloat
    {
        get
        {
            return frame.size.width
        }
        set
        {
            frame.size.width = newValue
        }
    }

    public var x: CGFloat
    {
        get
        {
            return frame.origin.x
        }
        set
        {
            frame.origin.x = newValue
        }
    }


    public var y: CGFloat
    {
        get
        {
            return frame.origin.y
        }
        set
        {
            frame.origin.y = newValue
        }
    }
}
