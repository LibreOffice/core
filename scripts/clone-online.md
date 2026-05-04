# clone-online.sh

A one-stop helper that gets you from "nothing on disk" to a working
Collabora Online build. It is written for people who do not work with
git, SSH, or build systems every day, so every step is explained as it
runs.

## What it does

Running the script walks you through 8 steps:

1. Checks that `git`, `ssh`, and `ssh-keygen` are installed.
2. Asks for your Gerrit username.
3. Finds your SSH key on this computer, or creates a new one for you.
4. Tests whether your SSH key already works with Gerrit.
5. If not, shows your public key and walks you through adding it to
   your Gerrit account in the browser.
6. Asks whether you want to build now (you can skip this).
7. Builds the engine (the document core) inside `engine/`.
8. Builds online on top of the engine.

You can stop after any step and re-run the script later. It will pick
up where you left off.

## Before you start

You need:

- A computer running Linux or macOS.
- A Collabora Gerrit account. If you do not have one, sign in once at
  <https://gerrit.collaboraoffice.com/> so the account is created.
- Your Gerrit username. You can see it on your profile page in
  Gerrit (top-right -> Settings -> Profile).
- Roughly 30 GB of free disk space if you plan to build.
- 1 to 3 hours of patience the first time you build (later builds are
  much faster because they are incremental).

You do NOT need to know any git commands, you do NOT need to generate
an SSH key by hand, and you do NOT need to read the Gerrit
documentation. The script handles all of it.

## Quick start

Since the script is what clones the repo, you need to download it
first. Open a terminal in the folder where you want the code to live
and run:

```sh
curl -fsSL "https://gerrit.collaboraoffice.com/plugins/gitiles/online/+/refs/heads/main/scripts/clone-online.sh?format=TEXT" \
    | base64 -d > clone-online.sh
chmod +x clone-online.sh
./clone-online.sh
```

That is it. The script is interactive: it asks you questions and
tells you exactly what to do, including what URL to open in your
browser.

## Choosing where the code goes

By default the script clones into a folder called `online` in the
current directory. To put it somewhere else, pass the path as the
first argument:

```sh
./clone-online.sh /path/where/i/want/online
```

## Skipping or forcing the build

The build is the slow part. You can control it with these flags:

| Flag             | What it does                                       |
| ---------------- | -------------------------------------------------- |
| (no flag)        | Asks "Build now? (Y/n)" after cloning.             |
| `--build`        | Builds without asking. Good for unattended runs.   |
| `--no-build`     | Stops after cloning.                               |
| `--skip-engine`  | Skips the engine build (use if engine is built).   |
| `--skip-online`  | Skips the online build.                            |

Examples:

```sh
# Just clone, do not build.
./clone-online.sh --no-build

# Clone and build everything without prompting.
./clone-online.sh --build

# Clone into /work/online and build.
./clone-online.sh /work/online --build

# Engine is already built; only build online.
./clone-online.sh --build --skip-engine
```

## What you will see

The script prints a clear header for each step, like this:

```
=== Step 3: Testing SSH connection to Gerrit ===
    SSH key already works with Gerrit. Skipping registration.
```

If something goes wrong, the message starts with `!!!` and is followed
by hints on what to try next.

## Step 5: adding your key to Gerrit

If your SSH key is not yet registered with Gerrit, the script will:

1. Print your public key in a clearly marked block, like:

   ```
   --- COPY EVERYTHING BETWEEN THESE LINES ---
   ssh-ed25519 AAAA...your-key... user@machine
   --- END ---
   ```

   On most systems it also copies the key to your clipboard, so you
   can paste it directly without selecting any text.

2. Tell you to open this page in your browser:

   <https://gerrit.collaboraoffice.com/settings/#SSHKeys>

3. Tell you, in plain English, where to paste the key and which
   button to click.

4. Wait for you to press Enter once you are done. Then it tests the
   connection again.

If the test still fails, the script prints common reasons (wrong
username, key not actually saved, port 29418 blocked) and exits so
you can fix the issue and re-run.

## Build prerequisites

The script does NOT install system packages. If the engine `autogen`
or `make` step complains about missing tools or libraries, install
the dependencies your distribution lists in
`engine/README.md` and re-run the script. The build stage is
incremental, so you do not lose progress.

Common starting points:

- Debian / Ubuntu: install `build-essential`, plus the `libreoffice`
  build dependency packages.
- Fedora: install `@development-tools` plus the `libreoffice` build
  deps.
- macOS: install Xcode command line tools and use Homebrew for the
  rest.

The dependency list is long and changes over time, so the script
cannot keep up with all distros.

## Re-running is safe

You can run the script as many times as you like:

- Existing SSH key: re-used, never overwritten.
- Working SSH connection: registration step is skipped.
- Existing clone in the target folder: clone step is skipped, build
  steps still run if you ask for them.
- `autogen.sh` and `make`: both are designed to be re-run. Make is
  incremental, so unchanged files are not rebuilt.

## Where things end up

After a successful run with `--build`:

- `<target>/`           the online source tree (top level).
- `<target>/engine/`    the document engine (core), grafted in.
- `<target>/engine/instdir/`  the engine build output.
- Online build artifacts are produced inside `<target>/`.

For how to actually start coolwsd and load a document, see
`<target>/docs/building.md` after the clone finishes.

## Troubleshooting

**"Required command not found: git"** (or ssh / ssh-keygen)
The script prints the install command for your platform. Run it,
then re-run the script.

**"Still cannot authenticate with Gerrit."**
Most often the Gerrit username is wrong, or the key was pasted into
the wrong field on the Gerrit settings page. Open the page again,
double-check the username under Profile, and re-add the key.

**"Engine autogen.sh failed."**
You are missing system packages. Read the actual error message; it
usually names the missing tool or library. Install it, then re-run
the script.

**"Online ./configure failed."**
The engine probably has not finished building, or `engine/instdir/`
is missing. Re-run without `--skip-engine` so the engine builds first.

**The build is taking forever.**
That is normal for the first run. The engine alone is typically 1 to
3 hours on a modern laptop. Subsequent builds are much faster.
