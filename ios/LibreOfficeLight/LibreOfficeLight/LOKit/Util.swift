//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import UIKit


func getDocumentsDirectory() -> URL
{
    let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
    return paths[0]
}

public extension CGRect
{
    public var desc: String
    {
        return "(x: \(self.origin.x), y: \(self.origin.y), width: \(self.size.width), height: \(self.size.height), maxX: \(self.maxX), maxY: \(self.maxY))"
    }
}

public func toString(_ pointer: UnsafeMutablePointer<Int8>?) -> String?
{
    if let p = pointer
    {
        return String(cString: p)
    }
    return nil
}

public func toString(_ pointer: UnsafePointer<Int8>?) -> String?
{
    if let p = pointer
    {
        return String(cString: p)
    }
    return nil
}

