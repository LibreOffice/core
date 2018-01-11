//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import Foundation


public typealias Runnable = () -> ()

/// Runs the closure on a queued background thread
public func runInBackground(_ runnable: @escaping Runnable)
{
    DispatchQueue.global(qos: .background).async(execute: runnable)
}


/// Runs the closure on the UI (main) thread. Exceptions are caught and logged
public func runOnMain(_ runnable: @escaping () -> ())
{
    DispatchQueue.main.async(execute: runnable)
}

/// Returns true if we are on the Main / UI thread
public func isMainThread() -> Bool
{
    return Thread.isMainThread
}

/// Runs tasks in a serial way on a single thread.
/// Why wouldn't we just use DispatchQueue or NSOperationQueue to do this?
/// Because neither guarantee running their tasks on the same thread all the time.
/// And in fact DispatchQueue will try and run sync tasks on the current thread where it can.
/// Both classes try and abstract the thread away, whereas we have to use the same thread, or we end up with deadlocks in LOKit
public class SingleThreadedQueue: Thread
{
    public init(name: String)
    {
        super.init()
        self.name = name
        self.start()
    }

    override public func main()
    {
        // You need the NSPort here because a runloop with no sources or ports registered with it
        // will simply exit immediately instead of running forever.
        let keepAlive = Port()
        let rl = RunLoop.current
        keepAlive.schedule(in: rl, forMode: .commonModes)

        rl.run()
    }

    /// Run the task on the serial queue, and return immediately
    public func async( _ runnable: @escaping Runnable)
    {
        let operation = BlockOperation {
            runnable()
        }
        async(operation: operation)
    }

    /// Run the task on the serial queue, and return immediately
    public func async( operation: Operation)
    {
        if ( Thread.current == self)
        {
            operation.start();
        }
        else
        {
            operation.perform(#selector(Operation.start), on: self, with: nil, waitUntilDone: false)
        }
    }

    public func sync<R>( _ closure: @escaping () -> R ) -> R
    {
        var ret: R! = nil
        let op = BlockOperation {
            ret = closure();
        }
        async(operation: op)
        op.waitUntilFinished()
        return ret
    }

}

