# Contributing to Collabora Online

Thanks for taking the time to contribute! The following is a set of
rules and guidelines for contributing to Collabora Online. Feel free
to propose changes to this document via Gerrit.

## Submitting issues

If you have questions about how to install or use Collabora Online,
please direct these to our [forum][forum]. If you have questions
about Collabora Online development, you may join us on [IRC][irc]
or [Telegram][telegram].

### Guidelines

- Please search the existing issues first; it is likely that your
  issue was already reported or even fixed.
  - Go to the main page of the repository, click "issues", and type
    any word in the top search/command bar.
  - You can also filter by appending e.g. "state:open" to the search
    string.
  - More info on [search syntax within github][gh-search].
- **SECURITY**: report any potential security bug to us following
  our [security policy][secpolicy] instead of filing an issue in our
  bug tracker.
- [Report the issue][report] using one of our templates; they
  include all the information we need to track down the issue.

Help us to maximize the effort we can spend fixing issues and adding
new features by not reporting duplicate issues.

[report]: https://github.com/CollaboraOnline/online/issues/new/choose
[forum]: https://forum.collaboraonline.com/
[irc]: https://web.libera.chat/?channels=cool-dev
[telegram]: https://t.me/CollaboraOnline
[gh-search]: https://help.github.com/articles/searching-issues
[secpolicy]: https://github.com/CollaboraOnline/online/security/policy

## Contributing source code

Thanks for wanting to contribute source code to Collabora Online.

Development and code review happen on our Gerrit instance at
**https://gerrit.collaboraoffice.com**. Pull requests on GitHub will
be automatically closed - please use Gerrit instead.

### First-time setup

For a guided setup that handles SSH key generation, Gerrit
registration, cloning, and the engine + online build, run:

```sh
curl -fsSL "https://gerrit.collaboraoffice.com/plugins/gitiles/online/+/refs/heads/main/scripts/clone-online.sh?format=TEXT" \
    | base64 -d > clone-online.sh
chmod +x clone-online.sh
./clone-online.sh
```

See [`scripts/clone-online.md`](scripts/clone-online.md) for the full
walkthrough and flags.

If you would rather set things up by hand:

1. Sign in to Gerrit at https://gerrit.collaboraoffice.com with
   your GitHub account.
2. Add your SSH key at
   https://gerrit.collaboraoffice.com/settings/#SSHKeys.
3. Clone the repo:
   ```bash
   git clone ssh://YOUR_USERNAME@gerrit.collaboraoffice.com:29418/online
   cd online
   ```
4. Build it ([on Linux][build-linux] or [on any platform][build-any]).

[build-linux]: https://www.collaboraonline.org/post/build-code/
[build-any]: https://forum.collaboraonline.com/t/start-developing-cool-on-any-platform-in-5-minutes/52

### Install the commit-msg hook

After cloning (whether via the script or by hand), install Gerrit's
commit-msg hook so every commit gets a Change-Id automatically:

```bash
scp -p -P 29418 YOUR_USERNAME@gerrit.collaboraoffice.com:hooks/commit-msg .git/hooks/
```

We use [Change-Ids][change-id] to track patches across rebases and
amendments.

[change-id]: https://gerrit-review.googlesource.com/Documentation/user-changeid.html

### Pick something to work on

Grab one of our newcomer-friendly [easy hacks][easyhacks].

[easyhacks]: https://www.collaboraonline.org/post/easyhacks/

### Submitting a patch

You can develop as normal and push your commits to Gerrit:

```bash
git push origin HEAD:refs/for/main
```

When you submit multiple patches at the same time, please consider
pushing like this:

```bash
git push origin HEAD~1:refs/for/main%wip
git push origin HEAD:refs/for/main
```

This way only the last patch triggers CI. Once CI passes on the top
commit, remove the WIP flag and add Verified +1 yourself to the rest
of the patches.

If you get stuck at any point, drop by one of our
[communication channels][channels].

[channels]: https://www.collaboraonline.org/post/communicate/

### Sign your work

We use the Developer Certificate of Origin (DCO) as an additional
safeguard for the Collabora Online project. This is a well
established and widely used mechanism to assure contributors have
confirmed their right to license their contribution under the
project's license.

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the appropriate version of
    the Mozilla Public License v2, or (for artwork) the Creative
    Commons Zero (CC0) license; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part by
    me, under the aforementioned licenses, in the appropriate
    version; or

(c) The contribution was provided directly to me by some other
    person who certified (a) or (b) and I have not modified it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    metadata and personal information I submit with it, including
    my sign-off) is maintained indefinitely and may be redistributed
    consistent with this project or the open source license(s)
    involved.

(e) I am granting this work to this project under the terms of both
    the Mozilla Public License v2 and the GNU Lesser General Public
    License version 3 as published by the Free Software Foundation.

(f) I agree that my contributions are adequately acknowledged by
    the "Copyright the Collabora Online contributors." header, and
    the public records of contribution.

If you can certify the above, add this line to every git commit
message:

```
Signed-off-by: Random J Developer <random@developer.example.org>
```

Use your real name and your real email address (sorry, no pseudonyms
or anonymous contributions). If you set your `user.name` and
`user.email` git configs, you can sign your commit automatically
with `git commit -s`.

### Commit messages

When reviewing a patch, we look for the following information in the
commit message:

- **Title**: a single line, short and to the point summary of what
  the patch does.
  - Rationale: to be able to do `git log --pretty=oneline` and have
    a usable result.

- **Intro**: observation of the current state.
  - Rationale: the problem to be solved is obvious to you, but not
    to the reviewer. It is good to have a list of steps to reproduce
    the problem.

- **Problem description**: pros and cons of the current state.
  - Rationale: when some feature does not work the way expected,
    frequently there is some other use case that motivated the
    current behavior. It is easier to not break the old use case
    with your change if you are aware of it.
  - If there was an old use case and you found it by research,
    please document it so the person reading the commit message
    finds it easily.

- **Solution**: give orders to the codebase.
  - A short description of how you introduce new behavior while not
    breaking old behavior is useful, because it may not be too
    obvious just by looking at what you changed.

An alternative is to have much of this information in a (public)
issue, refer to that issue, and have a short commit message. That
works better e.g. when using images to demonstrate the problem.

### Coding style

There is not really any serious rationale why the code ended up
being written in the style it is, but unless you plan to change some
style detail completely and consistently all over, please keep to
the style of the existing code when editing.

The style is roughly as follows, in rough order of importance:

- Source code files should have unix line terminators (LF).

#### C++

- As in Collabora Office, no hard TABs in source files. Only spaces.
  Indentation step is four columns.
- As in Collabora Office, the braces `{` and `}` of the block of `if`,
  `switch`, and `while` statements go on separate lines.
- Following Poco conventions, non-static member variables are
  prefixed with an underscore. Static members have a CamelCase name.
- Do use C++20. When writing new code, prefer the C++ Standard
  Library over Poco classes and functions.
- Always prefer the C++ wrapped version of a C library API. I.e.
  include `<cstring>` instead of `<string.h>`, use `std::memcpy()`
  instead of `memcpy()`, etc.
- Use `std::` prefix for all std API; do not ever do `using std;`.
  It is OK to use `using Poco::Foo;` all over (maybe not a good
  idea?), but please no `using` in headers.
- Member functions use `camelCaseWithInitialLowerCase` instead of
  `CamelCaseWithInitialUpperCase`.
- No kind of Hungarian prefixes.
- `return` is not a function but a statement; it does not need
  extra parentheses.
- Use `auto` in the following cases only:
  - iterators,
  - range-based for loops,
  - the type is spelled out in the same line already (e.g.
    initializing from a cast or a function that has a single type
    parameter).

  In other cases it makes the code more readable to still spell out
  the type explicitly.

#### JavaScript

- Indent code with TABs.
- For new code, use TypeScript.

### Security credential related changes

- Instead of the usual one, two reviews are needed.
- Instead of just choosing the 'approve' option on Gerrit, please
  add your explicit sign-off to the commit message when you review.

### For non-technical people

Hello, and thanks for stopping by! If you want to work on something
like an icon or a label fix but are not comfortable with the command
line, you can still contribute via GitHub:

1. [Fork the repo][fork] and make your changes.
2. Instead of opening a pull request, reach out to us on one of our
   [communication channels][channels] and a developer will help you
   submit the patch via Gerrit.

[fork]: https://github.com/CollaboraOnline/online/fork

### AI policy

We consider AI tools that help programming just one more tool that,
if used judiciously, can help developers learn to code. Volunteers
using such AI tools are permitted to get good ideas, but you are
responsible for the resulting code: you should be able to explain
each hunk of the diff you have written based on the generated
samples. Submitting patches without understanding their content,
copying AI generated code verbatim, or not testing the result before
submitting would create risk and waste reviewer time, and is
forbidden.

## Translations

Please submit translations via [Weblate][weblate].

[weblate]: https://hosted.weblate.org/projects/collabora-online
