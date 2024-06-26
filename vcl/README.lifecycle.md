# Understanding Transitional VCL Lifecycle

## How it used to look

All VCL classes were explicitly lifecycle managed; so you would do:

```
Dialog aDialog(...);   // old - on stack allocation
aDialog.Execute(...);
```

or:

```
Dialog *pDialog = new Dialog(...);  // old - manual heap allocation
pDialog->Execute(...);
delete pDialog;
```

or:

```
std::shared_ptr<Dialog> xDialog(new pDialog()); // old
xDialog->Execute(...);
// depending who shared the ptr this would be freed sometime
```

In several cases this lead to rather unpleasant code, when
various `shared_ptr` wrappers were used, the lifecycle was far less than
obvious. Where controls were wrapped by other ref-counted classes -
such as UNO interfaces, which were also used by native Window
pointers, the lifecycle became extremely opaque. In addition VCL had
significant issues with re-enterancy and event emission - adding
various means such as DogTags to try to detect destruction of a window
between calls:

```
ImplDelData aDogTag( this );	// 'orrible old code
Show( true, ShowFlags::NoActivate );
if( !aDogTag.IsDead() )         // did 'this' go invalid yet ?
    Update();
```

Unfortunately use of such protection is/was ad-hoc, and far
from uniform, despite the prevalence of such potential problems.

When a lifecycle problem was hit, typically it would take the
form of accessing memory that had been freed, and contained garbage due
to lingering pointers to freed objects.


## Where we are now

To fix this situation we now have a `VclPtr` - which is a smart
reference-counting pointer (`include/vcl/vclptr.hxx`) which is
designed to look and behave -very- much like a normal pointer
to reduce code-thrash. `VclPtr` is used to wrap all `OutputDevice`
derived classes thus:

```
VclPtr<Dialog> pDialog( new Dialog( ... ), SAL_NO_ACQUIRE );
...
pDialog.disposeAndClear();
```

However - while the `VclPtr` reference count controls the
lifecycle of the Dialog object, it is necessary to be able to
break reference count cycles. These are extremely common in
widget hierarchies as each widget holds (smart) pointers to
its parents and also its children.

Thus - all previous `delete` calls are replaced with `dispose`
method calls:

## What is dispose ?

Dispose is defined to be a method that releases all references
that an object holds - thus allowing their underlying
resources to be released. However - in this specific case it
also releases all backing graphical resources. In practical
terms, all destructor functionality has been moved into
`dispose` methods, in order to provide a minimal initial
behavioral change.

As such a `VclPtr` can have three states:

```
VclPtr<PushButton> pButton;
...
assert (pButton == nullptr || !pButton);    // null
assert (pButton && !pButton->isDisposed()); // alive
assert (pButton &&  pButton->isDisposed()); // disposed
```

## `ScopedVclPtr` - making disposes easier

While replacing existing code with new, it can be a bit
tiresome to have to manually add `disposeAndClear()`
calls to `VclPtr<>` instances.

Luckily it is easy to avoid that with a `ScopedVclPtr` which
does this for you when it goes out of scope.

## One extra gotcha - an initial reference-count of 1

In the normal world of love and sanity, eg. creating UNO
objects, the objects start with a ref-count of zero. Thus
the first reference is always taken after construction by
the surrounding smart pointer.

Unfortunately, the existing VCL code is somewhat tortured,
and does a lot of reference and de-reference action on the
class -during- construction. This forces us to construct with
a reference of 1 - and to hand that into the initial smart
pointer with a `SAL_NO_ACQUIRE`.

To make this easier, we have `Instance` template wrappers
that make this apparently easier, by constructing the
pointer for you.

### How does my familiar code change ?

Let's tweak the exemplary code above to fit the new model:

```
-	Dialog aDialog(... dialog params ... );
-	aDialog.Execute(...);
+	ScopedVclPtrInstance<Dialog> pDialog(... dialog params ... );
+	pDialog->Execute(...); // VclPtr behaves much like a pointer
```

or:

```
-	Dialog *pDialog = new Dialog(... dialog params ...);
+	VclPtrInstance<Dialog> pDialog(... dialog params ...);
	pDialog->Execute(...);
-	delete pDialog;
+	pDialog.disposeAndClear(); // done manually - replaces a delete
```

or:

```
-	std::shared_ptr<Dialog> xDialog(new Dialog(...));
+	ScopedVclPtrInstance<Dialog> xDialog(...);
	xDialog->Execute(...);
+	// depending how shared_ptr was shared perhaps
+	// someone else gets a VclPtr to xDialog
```

or:

```
-	VirtualDevice aDev;
+	ScopedVclPtrInstance<VirtualDevice> pDev;
```

Other things that are changed are these:

```
-	pButton = new PushButton(NULL);
+	pButton = VclPtr<PushButton>::Create(nullptr);
...
-	vcl::Window *pWindow = new PushButton(NULL);
+	VclPtr<vcl::Window> pWindow;
+       pWindow.reset(VclPtr<PushButton>::Create(nullptr));
```

### Why are these `disposeOnce` calls in destructors?

This is an interim measure while we are migrating, such that
it is possible to delete an object conventionally and ensure
that its dispose method gets called. In the 'end' we would
instead assert that a Window has been disposed in its
destructor, and elide these calls.

As the object's vtable is altered as we go down the
destruction process, and we want to call the correct dispose
methods we need this `disposeOnce();` call for the interim in
every destructor. This is enforced by a clang plugin.

The plus side of disposeOnce is that the mechanics behind it
ensure that a `dispose()` method is only called a single time,
simplifying their implementation.


### Who owns & disposes what?

Window sub-classes tend to create their widgets in one of two
ways and often both.

1. Derive from `VclBuilderContainer`. The `VclBuilder` then owns
   many of the sub-windows, which are fetched by a `get`
   method into local variables often in constructors eg.

```
VclPtr<PushButton> mpButton;  // in the class
, get(mpButton, "buttonName") // in the constructor
mpButton.clear();             // in dispose.
```

We only clear, not `disposeAndClear()` in our dispose method
for this case, since the `VclBuilder` / Container truly owns
this Window, and needs to dispose its hierarchy in the
right order - first children then parents.

2. Explicitly allocated Windows. These are often created and
   managed by custom widgets:

```
VclPtr<ComplexWidget> mpComplex;                     // in the class
, mpComplex( VclPtr<ComplexWidget>::Create( this ) ) // constructor
mpComplex.disposeAndClear();                         // in dispose
```

ie. an owner has to dispose things they explicitly allocate.

In order to ensure that the VclBuilderConstructor
sub-classes have their Windows disposed at the correct time
there is a `disposeBuilder();` method - that should be added
-only- to the class immediately deriving from
`VclBuilderContainer`'s dispose.

### What remains to be done?

* Expand the `VclPtr` pattern to many other less
  than safe VCL types.

* create factory functions for `VclPtr<>` types and privatize
  their constructors.

* Pass `const VclPtr<> &` instead of pointers everywhere
  + add `explicit` keywords to VclPtr constructors to
    accelerate compilation etc.

* Cleanup common existing methods such that they continue to
  work post-dispose.

* Dispose functions should be audited to:
  + not leave dangling pointsr
  + shrink them - some work should incrementally
    migrate back to destructors.

* `VclBuilder`
  + ideally should keep a reference to pointers assigned
    in `get()` calls - to avoid needing explicit `clear`
    code in destructors.

## FAQ / debugging hints

### Compile with dbgutil

This is by far the best way to turn on debugging and
assertions that help you find problems. In particular
there are a few that are really helpful:

```
vcl/source/window/window.cxx (Window::dispose)
"Window ( N4sfx27sidebar20SidebarDockingWindowE (Properties))
          ^^^ class name                 window title ^^^
with live children destroyed:  N4sfx27sidebar6TabBarE ()
N4sfx27sidebar4DeckE () 10FixedImage ()"
```

You can de-mangle these names if you can't read them thus:

```
$ c++filt -t N4sfx27sidebar20SidebarDockingWindowE
sfx2::sidebar::SidebarDockingWindow
```

In the above case - it is clear that the children have not been
disposed before their parents. As an aside, having a dispose chain
separate from destructors allows us to emit real type names for
parents here.

To fix this, we will need to get the dispose ordering right,
occasionally in the conversion we re-ordered destruction, or
omitted a `disposeAndClear()` in a `::dispose()` method.

- If you see this, check the order of `disposeAndClear()` in
   the `sfx2::Sidebar::SidebarDockingWindow::dispose()` method

- also worth `git grep`ing for `new sfx::sidebar::TabBar` to
   see where those children were added.

### Check what it used to do

While a ton of effort has been put into ensuring that the new
lifecycle code is the functional equivalent of the old code,
the code was created by humans. If you identify an area where
something asserts or crashes here are a few helpful heuristics:

* Read the `git log -u -- path/to/file.cxx`

### Is the order of destruction different?

In the past many things were destructed (in reverse order of
declaration in the class) without explicit code. Some of these
may be important to do explicitly at the end of the destructor.

eg. having a `Idle` or `Timer` as a member, may now need an
   explicit `.Stop()` and/or protection from running on a
   disposed Window in its callback.

### Is it `clear` not `disposeAndClear`?

sometimes we get this wrong. If the code previously used to
use `delete pFoo;` it should now read `pFoo->disposeAndClear();`.
Conversely if it didn't delete it, it should be `clear()` it
is by far the best to leave disposing to the `VclBuilder` where
possible.

In simple cases, if we allocate the widget with `VclPtrInstance`
or `VclPtr<Foo>::Create` - then we need to `disposeAndClear` it too.

### Event / focus / notification ordering

In the old world, a large amount of work was done in the
`~Window` destructor that is now done in `Window::dispose`.

Since those Windows were in the process of being destroyed
themselves, their vtables were adjusted to only invoke Window
methods. In the new world, sub-classed methods such as
`PreNotify`, `GetFocus`, `LoseFocus` and others are invoked all down
the inheritance chain from children to parent, during dispose.

The easiest way to fix these is to just ensure that these
cleanup methods, especially LoseFocus, continue to work even
on disposed Window sub-class instances.

### It crashes with some invalid memory...

Assuming that the invalid memory is a Window sub-class itself,
then almost certainly there is some cockup in the
reference-counting; eg. if you hit an `OutputDevice::release`
assert on `mnRefCount` - then almost certainly you have a
Window that has already been destroyed. This can easily
happen via this sort of pattern:

```
Dialog *pDlg = VclPtr<Dialog>(nullptr /* parent */);
// by here the pDlg quite probably points to free'd memory...
```

It is necessary in these cases to ensure that the `*pDlg` is
a `VclPtr<Dialog>` instead.

### It crashes with some invalid memory #2...

Often a `::dispose` method will free some `pImpl` member, but
not `NULL` it; and (cf. above) we can now get various `virtual`
methods called post-dispose; so:

a) `delete pImpl; pImpl = NULL; // in the destructor`
b) `if (pImpl && ...)           // in the subsequently called method`
