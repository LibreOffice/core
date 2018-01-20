//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import Foundation
import UIKit


open class CachedRender
{
    open let part: Int32
    open let canvasSize: CGSize
    open let tileRect: CGRect
    open let image: UIImage

    public init(part: Int32, canvasSize: CGSize, tileRect: CGRect, image: UIImage)
    {
        self.canvasSize = canvasSize
        self.tileRect = tileRect
        self.image = image
        self.part = part
    }
}

class RenderCache
{
    let CACHE_LOWMEM = 4
    let CACHE_NORMAL = 20

    var cachedRenders: [CachedRender] = []
    var hasReceivedMemoryWarning = false

    let lock = NSRecursiveLock()

    func emptyCache()
    {
        lock.lock(); defer { lock.unlock() }
        cachedRenders.removeAll()
    }

    func pruneCache()
    {
        lock.lock(); defer { lock.unlock() }

        let max = hasReceivedMemoryWarning ? CACHE_LOWMEM : CACHE_NORMAL
        while cachedRenders.count > max
        {
            cachedRenders.remove(at: 0)
        }
    }

    func add(cachedRender: CachedRender)
    {
        lock.lock(); defer { lock.unlock() }

        cachedRenders.append(cachedRender)
        pruneCache()
    }

    func get(part: Int32, canvasSize: CGSize, tileRect: CGRect) -> UIImage?
    {
        lock.lock(); defer { lock.unlock() }

        if let cr = cachedRenders.first(where: {
                $0.canvasSize == canvasSize
                && $0.tileRect == tileRect
                && $0.part == part
        })
        {
            return cr.image
        }
        return nil
    }
}
